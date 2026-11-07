// standard-version postchangelog hook.
//
// standard-version rewrites everything above the latest release heading in
// CHANGELOG.md, so notes written there by hand are lost. Instead, notes for the
// next release go in CHANGELOG.next.md. This hook inserts them under the
// release heading standard-version just wrote, then deletes CHANGELOG.next.md
// so the following release starts with no notes.
//
// With no CHANGELOG.next.md, or an empty one, it does nothing.

const fs = require("fs");
const { execFileSync } = require("child_process");

const changelogFile = "CHANGELOG.md";
const notesFile = "CHANGELOG.next.md";
const releaseHeading = /^#+ \[?\d+\.\d+\.\d+.*$/m;

function removeNotesFile() {
  try {
    // Stage the deletion so it lands in the release commit (needs commitAll).
    execFileSync("git", ["rm", "-q", notesFile], { stdio: "pipe" });
  } catch (_err) {
    // Not tracked by git: just delete it.
    fs.unlinkSync(notesFile);
  }
}

if (!fs.existsSync(notesFile)) {
  process.exit(0);
}

const notes = fs.readFileSync(notesFile, "utf8").trim();

if (notes) {
  const changelog = fs.readFileSync(changelogFile, "utf8");
  const match = changelog.match(releaseHeading);

  if (!match) {
    console.error(`no release heading found in ${changelogFile}`);
    process.exit(1);
  }

  const end = match.index + match[0].length;
  const updated =
    changelog.slice(0, end) +
    "\n\n### Notes\n\n" +
    notes +
    "\n" +
    changelog.slice(end);

  fs.writeFileSync(changelogFile, updated);
}

removeNotesFile();
