const child_process = require("child_process");
const fs = require("fs");

const file = fs.readFileSync(process.argv[2], "utf8");
console.log(
  file.replace(
    / \(supertuxkart \+ (0x[0-9a-f]{1,16})\)/g,
    (existing, offset) =>
      existing +
      " => " +
      child_process
        .execFileSync("aarch64-none-elf-addr2line", [
          "-e",
          "./bin/supertuxkart",
          "-f",
          "-p",
          "-C",
          "-a",
          offset,
        ])
        .toString("utf8")
        .trim()
  )
);
