---
description: Read-only review of firmware changes against firmware/CODE_STYLE.md (naming, units, defines). Run alongside firmware-reviewer before committing.
mode: subagent
permissions:
  - action: edit
    resource: "*"
    effect: deny
  - action: shell
    resource: "*"
    effect: deny
  - action: shell
    resource: "git diff *"
    effect: allow
  - action: shell
    resource: "git status *"
    effect: allow
  - action: shell
    resource: "git log *"
    effect: allow
  - action: shell
    resource: "git show *"
    effect: allow
  - action: shell
    resource: "grep *"
    effect: allow
  - action: shell
    resource: "head *"
    effect: allow
  - action: shell
    resource: "tail *"
    effect: allow
  - action: shell
    resource: "wc *"
    effect: allow
  - action: shell
    resource: "sed -n *"
    effect: allow
---

Review the uncommitted changes in this repo against the firmware code style guide at `firmware/CODE_STYLE.md` — read it first, it is the spec. Scope: hand-written files (`firmware/app/`, and `USER CODE` blocks in `Core/`). CubeMX-generated code (`Core/` outside USER CODE blocks, `Drivers/`) is exempt from this review.

Shell is allow-listed: only bare `git diff`/`git status`/`git log`/`git show` work — no pipes, no `git -C` — so use the read/grep/glob tools for file inspection.

Check the changed code against every rule in the guide:

1. **Files**: lower snake case; headers beside sources in `firmware/app/`.
2. **Functions**: `module_subject_verb` lower snake case, module prefix on every function including statics; `static` unless public API.
3. **Variables**: lower snake case; physical units appended to the name (`_ms`, `_ticks`, `_mv`, `_kph`, `_m`, ...); booleans read as predicates.
4. **Types**: PascalCase; enum members and defines ALL_CAPS with underscores and units.
5. **Defines**: all at the top of the file after the includes; no magic numbers in logic (scale-factor suffixes like `X1000` allowed with a comment).
6. **Includes**: own header first, then project, then HAL/vendor, then libc; groups separated by blank lines.
7. **Comments**: `//` line comments explaining why; `TODO:` markers; big tables inside `// clang-format off/on`.
8. **State**: no new extern globals beyond CubeMX peripheral handles; `volatile` discipline if an ISR appears.

Only report violations in the CHANGED code (the guide is new; do not demand retroactive renames of untouched code unless the change itself touches that line). Do not report formatting issues clang-format would fix — only check `firmware/check.sh` output for that. Report findings in severity order with file:line references and the rule from the guide they violate. If there are none, say so.
