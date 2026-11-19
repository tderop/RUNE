# RUNE User Guide
Thank you for using RUNE! Instructons on how to install and use it are all listed for you below.

## Installation
> [!NOTE]
> Atmosphére custom firmware is required to run homebrew software like RUNE on your Nintendo Switch. For information on how to install Atmosphére, please visit [switch.hacks.guide](https://switch.hacks.guide/) and follow the guides.

1. Download the latest standalone `.nro` release of RUNE
2. Copy `RUNE.nro` to `sdmc:/switch/RUNE.nro` on your Nintendo Switch's SD card
3. Boot into Atmosphére
4. Click on a game to launch it, and hold R whilst you select a user
5. Continue holding R until the **hbmenu** appears, then select RUNE

You're now running RUNE on your Nintendo Switch!

### Why don't I see icons in RUNE?
Because of copyright law, RUNE is not allowed to redistribute content owned by Nintendo Co., Ltd., which includes the item icons you see in your inventory.<br><br>Fortunately, this doesn't mean you _can't_ have icons, it just means you need to **open RUNE in "title takeover" mode** with Breath of the WIld selected. **This takes time**, on average about 1 minute and 30 seconds. You only need to do this once.<br><br>By doing this, you give RUNE access to the files in your local Breath of the Wild installation - allowing it to extract the menu icons to your SD card. The icons are cached in `sdmc:/switch/RUNE/assets/thumb`; take a look - they're extracted from your own copy of the game! All of the icons combined should take up a total of no more than ~40MB. You're responsible for keeping these on your own device! Redistributing these assets to anyone else is a violation of copyright law.
<br>

## Usage and Troubleshooting

### Launching
When launching RUNE, you need to launch it in **title takeover** mode. If you don't, you may get this old-looking error:

![RUNE failing to launch due to applet mode](screenshots/launchfail.png)

This means you launched RUNE through the Album applet. RUNE cannot run in applet mode, as it does not provide enough resources for RUNE to run stably. Follow the on-screen instructions and your issue should be resolved.

### Loading
During load, RUNE shows a logo and provides some information on the loading progress in the bottom left. It's likely this succeeds and RUNE loads successfully, however you could encounter this error:

![RUNE failing to launch due to missing data](screenshots/splasherror.png)

This means RUNE either failed to access your Breath of the Wild save data, or the account that you opened RUNE with does not have any save data for Breath of the Wild yet. In this case, you need to open the game on the account you desire to use, and then ensure there is at least one save slot before re-opening RUNE. 

### Editing a Save
After launching RUNE, you will see a save select menu, similar to the in-game one.

![RUNE save select menu](screenshots/select.png)

In the top right, the version number is the **version of your game save** (more specifically the version of the save slot at index 0, as save version can vary between slots...)

Selecting a save will open the editor. If the editor fails to load - or crashes RUNE - it is possible that the selected save slot is corrupted or unsupported.

The editor looks something like this:

![RUNE save editor](screenshots/editor1.png)

In the stats tab, you can edit a few common values. Clicking on any of the options here will open an overlay that will let you edit the value. Some overlays will show a **question mark** (?) button in the top left. When clicked, these buttons will display extra information on screen to explain some details about the value you're modifying.

You can use L and R to navigate between top menus (Stats, Coordinates and Completionism in this case)

The equipment and items tabs open menus that looks like this:

![RUNE equipment editor](screenshots/editor2.png)

It is designed to look similar to the in-game equipment inventory. In this case, however, clicking on an item opens an overlay that will provide you with a list of properties you can edit.
These properties range from durability, to modifier, to armor upgrade level, to dye colour, and more. Some modifiers will show a "modifier value" editor, as these modifiers require a value to know "how much" to modify the equipment.

You can navigate pages in inventory menus using the right stick.

The final tab is the **Dev Editor**. Only use this one if you **know what you are doing**. It is designed to let you edit any value in save data freely, acting as a simple middleman between you and the save file's raw binary data. **Modifying values here could result in a corrupted state / softlocked save**; it is entirely for people that know what they are doing, or just want to mess around with a save file they don't particularly care about.

![RUNE developer editor](screenshots/deveditor.png)

You can search this tab using "X", which will open a keyboard for you to type a search query. If your search matches no keys in save data, the menu will just display all available keys, like above.