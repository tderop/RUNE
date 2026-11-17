# RUNE Technical Debt and Issues
This is a list of RUNE's technical weaknesses, design flaws, and known issues to be used to inform development.

## 1. The Widget Building System is Poorly Designed
All `EditorWidget` instances need to execute a builder function to consider themselves loaded and valid. More complex widgets - such as the tile widget used in inventory-editing menus - often need to perform tasks that would make the code virtually unmaintainable if written using exclusively a widget building function (which is why tiles have their own class, `Rune::Inventory::Tile`!)<br>
The widget system should be migrated to an entirely object-oriented system, in which different widget types are derived from the primary `EditorWidget` class, in order to have a maintainable editor codebase.

### The Menu System is Poorly Designed
For the same reason, with the same remedial actions.

## 2. Save Data is Directly Modified by Widgets
When modifying a value through an `EditorWidget`, the widgets themselves are responsible for validation checks, which they shouldn't be as these are primarily designed to be for UI interaction only.<br>
The system should be migrated to a database-like model, in which edits are validated and saved through a setter, and changes are logged. 

## 3. ~~Inventory Menus are Slow~~
~~Inventory menus were implemented rapidly in order to get the base functionality in place. Therefore they are slow to load and can feel rough to use. Icon generation and metadata reading are implemented inefficiently and need to be optimised significantly to bring inventory generation as close as possible to a target of 100ms.~~
After a good amount of refactoring, I'm quite satisfied with the speed of inventory menu generation! Feedback is what counts now.

## 4. Inconsistent Private Member Variable Naming 
Member variables do not use the same naming convention amongst different classes. Some use an underscore before the name, some use a trailing underscore, and some have no underscores despite being private. This can introduce confusion when trying to differentiate between private and public members and methods when programming/reading code.