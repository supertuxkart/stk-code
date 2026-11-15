// Exercise unfinished games through the visible browser controls, including
// pause ownership, cancelling confirmation, fullscreen, final saves, and replay.
import assert from 'node:assert/strict';
import { existsSync, mkdirSync, writeFileSync } from 'node:fs';
import { createRequire } from 'node:module';
import { fileURLToPath } from 'node:url';
const require = createRequire(new URL('../build-web-tools/browser-test/package.json', import.meta.url));
const { chromium } = require('playwright');
const artifacts = new URL('../build-web-tools/browser-test/quit-during-play/', import.meta.url);
mkdirSync(artifacts, { recursive: true });
const chrome = '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome';
const browser = await chromium.launch({
  executablePath: process.env.PLAYWRIGHT_CHROMIUM_EXECUTABLE_PATH || (existsSync(chrome) ? chrome : undefined),
  headless: true,
  args: ['--enable-webgl', '--use-angle=swiftshader', '--enable-unsafe-swiftshader'],
});
const page = await browser.newPage({ viewport: { width: 1200, height: 900 } });
page.setDefaultTimeout(120000);
const errors = [];
page.on('pageerror', error => errors.push(error.message));
let navigations = 0;
page.on('framenavigated', frame => { if (frame === page.mainFrame()) navigations++; });
async function assertClock(frame, paused) {
  await page.waitForTimeout(400);
  // Check actual simulation time; menu animations and GPU dithering can change
  // screenshot pixels while the race is correctly paused and its HUD is hidden.
  const before = await frame.evaluate(() => Module._browser_play_time());
  await page.waitForTimeout(1000);
  const after = await frame.evaluate(() => Module._browser_play_time());
  assert.ok(before >= 0 && after >= 0);
  if (paused) assert.equal(after, before, 'Race clock must stay frozen');
  else assert.ok(after > before, 'Race clock must advance');
}
try {
  await page.goto(process.env.GAME_URL || 'http://localhost:8080', { waitUntil: 'domcontentloaded' });
  await page.waitForFunction(() => !document.getElementById('play').disabled);
  const initialNavigations = navigations;
  for (const [mode, args] of [
    ['race', ['--race-now', '--track=olivermath', '--kart=tux', '--numkarts=1', '--laps=3', '--use-keyboard=0']],
    ['battle', ['--race-now', '--mode=2', '--track=battleisland', '--kart=tux', '--numkarts=2', '--use-keyboard=0']],
  ]) {
    const frame = page.frames().find(frame => frame.url().endsWith('/game.html'));
    // Start a deterministic activity using the native CLI, while keeping the
    // launcher's real Play click and the normal runtime lifecycle intact.
    await frame.evaluate(args => {
      const start = window.startGame;
      window.startGame = () => start(args);
    }, args);
    await page.click('#play');
    await page.waitForFunction(() => document.getElementById('status').textContent.startsWith('Game running.'));
    await frame.waitForFunction(() => !document.getElementById('pause').disabled);
    console.log(`${mode}: unfinished activity started`);

    await page.click('#fullscreen');
    await page.waitForFunction(() => document.fullscreenElement?.id === 'game');
    await frame.click('#pause');
    await page.screenshot({ path: fileURLToPath(new URL(`${mode}-pause-menu.png`, artifacts)) });
    if (mode === 'race') await assertClock(frame, true);

    // Cancelling from an already-paused game must leave its native menu paused.
    await frame.click('#quit');
    await frame.waitForFunction(() => document.getElementById('quit-dialog').open);
    await frame.click('#cancel-quit');
    if (mode === 'race') await assertClock(frame, true);
    await frame.click('#pause');
    if (mode === 'race') await assertClock(frame, false);

    // Quit during live gameplay pauses it; Escape cancels and resumes it.
    await frame.click('#quit');
    await frame.waitForFunction(() => document.getElementById('quit-dialog').open);
    assert.equal(await frame.evaluate(() => document.activeElement.id), 'cancel-quit');
    if (mode === 'race') await assertClock(frame, true);
    await page.keyboard.press('Escape');
    await frame.waitForFunction(() => !document.getElementById('quit-dialog').open);
    if (mode === 'race') await assertClock(frame, false);
    // Browsers may use Escape to leave fullscreen as well; re-enter for quit.
    if (!(await page.evaluate(() => !!document.fullscreenElement))) {
      await page.click('#fullscreen');
      await page.waitForFunction(() => document.fullscreenElement?.id === 'game');
    }
    await frame.click('#quit');
    await frame.waitForFunction(() => document.getElementById('quit-dialog').open);
    await page.screenshot({ path: fileURLToPath(new URL(`${mode}-confirm.png`, artifacts)) });
    await frame.evaluate(value => Module.FS.writeFile('/persist/unfinished-quit.txt', value), mode);
    await frame.click('#confirm-quit');
    await page.waitForFunction(() => !document.getElementById('play').disabled &&
      document.getElementById('play').textContent === 'Play again');
    assert.equal(await page.evaluate(() => document.fullscreenElement), null);
    const next = page.frames().find(frame => frame.url().endsWith('/game.html'));
    assert.equal(await next.evaluate(() => Module.FS.readFile('/persist/unfinished-quit.txt', { encoding: 'utf8' })), mode);
    assert.equal(navigations, initialNavigations);
    console.log(`${mode}: pause/cancel/quit passed, saved, exited fullscreen, replay ready without reloading`);
  }
  assert.deepEqual(errors, []);
} catch (error) {
  await page.screenshot({ path: fileURLToPath(new URL('failure.png', artifacts)) }).catch(() => {});
  writeFileSync(new URL('failure.log', artifacts), (await page.locator('#log').textContent()) || '');
  throw error;
} finally {
  await browser.close();
}
