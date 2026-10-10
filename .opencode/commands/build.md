---
description: Strict Debug + Release build (same as CI) with memory usage
---

Strict build output (`speedo-code/check.sh`, -Werror):

!`speedo-code/check.sh 2>&1; echo "exit status: $?"`

If it failed, find the cause and fix it, then re-run `speedo-code/check.sh`. If it passed, just report flash/RAM usage in one line.
