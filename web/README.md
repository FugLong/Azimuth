# Portal Source Assets

This directory is the source of truth for the firmware portal UI.

## Workflow

1. Edit:
   - `web/index.html` (markup / structure)
   - `web/styles.css` (styles)
   - `web/app/main.js` (portal runtime logic)

2. Regenerate embedded firmware asset:

```bash
python3 scripts/portal_codegen.py --generate
```

The generator inlines local CSS/JS assets into the embedded HTML payload, then writes
`src/portal_html.cpp` deterministically for clean diffs.
