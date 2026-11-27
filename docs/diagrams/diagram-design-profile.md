<!-- diagram-design-profile
name: rave-compass
slug: rave-compass
created: 2026-11-27
updated: 2026-11-27
notes: Linen/sumi tokens from the author's design language; system font stack only (no network fonts). Referenced by ../../.diagram-design. To use it, copy this file to ~/.diagram-design/profiles/rave-compass.md.
-->
# Style Guide (rave-compass)

Font source: `system`

| Role | Light | Dark |
|---|---|---|
| `paper` | `#F7F5F2` | `#1C1B19` |
| `ink` | `#2C2C2A` | `#E8E5E1` |
| `muted` | `#5C5650` | `#AEA9A2` |
| `soft` | `#6E6963` | `#9A9590` |
| `accent` (focal, 1-2 per diagram) | `#A8482D` | `#D06A4C` |
| `link` (over-the-air / external) | `#3B4F5E` | `#8FAFC2` |
| node `backend` fill | `#FFFFFF` | `#242320` |

Washes: `ink @ 0.02-0.05` for zones/stores, `accent @ 0.08` for focal fill.

| Role | Family stack |
|---|---|
| `title` / `callout` | `'Iowan Old Style', Georgia, 'Times New Roman', serif` |
| `node-name` | `system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif` |
| `sublabel` / `eyebrow` / `arrow-label` | `ui-monospace, 'SF Mono', Menlo, Consolas, monospace` |

Exports: `python3 -I <skill>/scripts/export_svg.py docs/diagrams/<slug>.html docs/diagrams/<slug>.svg --system-fonts`.
