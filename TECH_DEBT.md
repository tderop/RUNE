# RUNE Technical Debt and Issues
This is a list of RUNE's technical weaknesses, design flaws, and known issues to be used to inform development.

## 1. The Widget Building System is Poorly Designed
All `EditorWidget` instances need to execute a builder function to consider themselves loaded and valid. More complex widgets - such as the tile widget used in inventory-editing menus - often need to perform tasks that would make the code virtually unmaintainable if written using exclusively a widget building function (which is why tiles have their own class, `Rune::Inventory::Tile`!)<br>
The widget system should be migrated to an entirely object-oriented system, in which different widget types are derived from the primary `EditorWidget` class, in order to have a maintainable editor codebase.

### The Menu System is Poorly Designed
For the same reason, with the same remedial actions.


