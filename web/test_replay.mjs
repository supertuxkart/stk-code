// Real-browser regression: quit the native game, then start it again without
// navigating the launcher. Includes fullscreen exit and a final-save roundtrip.
// Setup/run instructions are in web/README.md; Playwright stays in ignored tools.
import assert from 'node:assert/strict';
import { existsSync, mkdirSync, writeFileSync } from 'node:fs';
import { createRequire } from 'node:module';
import { fileURLToPath } from 'node:url';
const require = createRequire(new URL('../build-web-tools/browser-test/package.json', import.meta.url));
const { chromium } = require('playwright');
const artifacts = new URL('../build-web-tools/browser-test/replay/', import.meta.url);
mkdirSync(artifacts, { recursive: true });
const chrome = '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome';
const browser = await chromium.launch({
  executablePath: process.env.PLAYWRIGHT_CHROMIUM_EXECUTABLE_PATH || (existsSync(chrome) ? chrome : undefined),
  headless: true,
  args: ['--enable-webgl', '--use-angle=swiftshader', '--enable-unsafe-swiftshader'],
});
const context = await browser.newContext({ viewport: { width: 1200, height: 900 } });
const page = await context.newPage();
page.setDefaultTimeout(180000);
let navigations = 0;
const errors = [];
page.on('framenavigated', frame => { if (frame === page.mainFrame()) navigations++; });
page.on('pageerror', error => errors.push(error.message));
try {
  await page.goto(process.env.GAME_URL || 'http://localhost:8080', { waitUntil: 'domcontentloaded' });
  await page.waitForFunction(() => !document.getElementById('play').disabled);
  console.log('Initial game session ready');
  const initialNavigations = navigations;
  for (let cycle = 1; cycle <= 3; cycle++) {
    const oldFrame = page.frames().find(frame => frame.url().endsWith('/game.html'));
    assert.ok(oldFrame, 'Game session was prepared');
    if (cycle > 1) {
      const saved = await oldFrame.evaluate(() => Module.FS.readFile('/persist/replay-check.txt', { encoding: 'utf8' }));
      assert.equal(saved, String(cycle - 1), 'The previous quit saved before replacing the frame');
    }
    await page.click('#play');
    await page.waitForFunction(() => document.getElementById('status').textContent.startsWith('Game running.'));
    console.log(`Cycle ${cycle}: game started`);
    if (cycle === 2) {
      await page.click('#fullscreen');
      await page.waitForFunction(() => document.fullscreenElement?.id === 'game');
    }
    // This fresh test context has no player yet: Create User's red Exit Game
    // button is at these native canvas coordinates. Click the real game UI,
    // rather than faking the runtime's onExit callback or lifecycle messages.
    const box = await oldFrame.locator('#canvas').boundingBox();
    assert.ok(box);
    await oldFrame.evaluate(value => Module.FS.writeFile('/persist/replay-check.txt', String(value)), cycle);
    await page.mouse.click(box.x + box.width * 386 / 960, box.y + box.height * 470 / 540);
    console.log(`Cycle ${cycle}: clicked the native Exit Game button`);
    await page.waitForFunction(() => !document.getElementById('play').disabled &&
      document.getElementById('play').textContent === 'Play again');
    assert.equal(await page.evaluate(() => document.fullscreenElement === null), true);
    assert.equal(await page.locator('#game').isVisible(), false);
    assert.equal(await page.locator('#welcome').isVisible(), true);
    assert.notEqual(page.frames().find(frame => frame.url().endsWith('/game.html')), oldFrame);
    assert.equal(navigations, initialNavigations, 'The launcher must not reload');
    await page.screenshot({ path: fileURLToPath(new URL(`quit-${cycle}.png`, artifacts)) });
    console.log(`Cycle ${cycle}: native quit → fullscreen cleared → Play again ready; launcher did not reload`);
  }
  assert.deepEqual(errors, [], 'No uncaught browser errors');
} catch (error) {
  await page.screenshot({ path: fileURLToPath(new URL('failure.png', artifacts)) }).catch(() => {});
  writeFileSync(new URL('failure.log', artifacts), (await page.locator('#log').textContent()) || '');
  throw error;
} finally {
  await browser.close();
}
