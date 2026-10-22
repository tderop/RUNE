#include <Screens/Editor.hpp>

namespace Rune{
    namespace Editor{
        EditorTabData TabData = {
            {
                "Stats",
                EditorTab{
                    .menus={"Stats", "Coordinates", "Completionism"},
                    .menuEntries={
                        {
                            "Stats",
                            {
                                //////////////////////////
                                // "<PAD>",
                                //////////////////////////
                                "CurrentRupee", // Rupee balance
                                "CurrentMamo",  // Mons balance
                                //////////////////////////
                                "<PAD>",
                                //////////////////////////
                                // "dummy_float",
                                "MaxHartValue",
                                "StaminaMax",
                                "MiniGame_BirdMan_HillTower_RecordDistance",
                                //////////////////////////
                                "<PAD>",
                                //////////////////////////
                                "ActorName_SeakSensor_Slot0"
                                
                            },
                        },
                        {
                            "Coordinates",
                            {
                                "PlayerSavePos",
                                "Horse_Pos",
                                //////////////////////////
                                "<PAD>",
                                //////////////////////////
                                "WarpDLC_DestPos",
                                "ShootingStarPosition"
                            },
                        },
                        {
                            "Completionism",
                            {
                                "PlayReport_PlayTime",
                                "HiddenKorok_Number",
                                //////////////////////////
                                "<PAD>",
                                //////////////////////////
                                "DefeatedSandwormNum",
                                "DefeatedForestGiantNum",
                                "DefeatedGolemNum",
                                //////////////////////////
                                "<PAD>",
                                //////////////////////////
                                "IsGet_Obj_Motorcycle" // Motorcycle Obtained
                            }
                        }
                    }
                }
            },

            {
                "Equipment",
                EditorTab{
                    .menus={"Weapons", "Bows", "Shields", "Armor"},
                    .menuEntries={}
                }
            },

            {
                "Items",
                EditorTab{
                    .menus={"Materials", "Food", "Key Items"},
                    .menuEntries={}
                }
            },

            {
                "Dev Editor",
                EditorTab{
                    .menus={"Dev Editor"},
                    .menuEntries={}
                }
            }
        };
    }
}