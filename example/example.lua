  set_background("background.png", "ground_map.png")
  set_character ("test_character.png", "indy_head.png", 800, 470)
coffre_fort_verrouille = true
coffre_fort_ouvert = false
clef_usb_inseree = false


-- character =
--   {
--     skin = "sophie.png",
--     coordinates = {10, 10}
--   }

-- scene_objects =
--   {
--     {
--       id = "coffre_fort",
--       skin = "coffre_fort.png",
--       name = "Coffre fort",
--       description = say("Je ne sais pas ce qu'il garde dans ce coffre-fort…"),
--       actions =
--         {
--           {
--             name = "Ouvrir",
--             effect = function()
--               open_window("coffre_fort_code")
--             end
--           },
--         }
--     },

--     {
--       id = "ordinateur",
--       skin = "ordinateur.png",
--       name = "Ordinateur d'Harpagon",
--       description = "La session est ouverte… Pour la sécurité, on repassera.",
--       actions =
--         {
--           {
--             name = "Utiliser",
--             effect = say("Je n'ai rien à y faire.")
--           }
--         }
--     },

--     {
--       id = "bouteille",
--       skin = "bouteille.png",
--       name = "Bouteille d'eau",
--       description = "Pas moyen de lui faire utiliser une gourde… Il adore son plastique.",
--       actions =
--         {
--           {
--             name = "Prendre",
--             effect = pick("bouteille")

--           },
--           {
--             name = "Pousser",
--             effect = say("Je n'ai pas envie d'en renverser sur le clavier…")
--           }
--         }
--     }
--   }

-- inventory_objects =
--   {
--     {
--       id = "telephone",
--       skin = "telephone.png",
--       name = "Téléphone",
--       on = true,
--       actions = function()
--         say("Je vais essayer d'appeler Harpagon.")
--         animate("sophie_phone.png")
--         say("Ça ne répond pas.")
--       end
--     },
    
--     {
--       id = "bouteille",
--       skin = "bouteille_inventaire.png",
--       name = "Bouteille d'eau",
--       on = true,
--       actions =
--         {
--           {
--             target = "ordinateur",
--             effect = say("Pour me faire virer ?")
--           },

--           {
--             target = "plante",
--             effect = function()
--               animate("verse_eau.png")
--               say("Voilà, elle est arrosée.")
--             end
--           }
--         }
--     }
--   }

-- windows =
--   {
--     id = "code_coffre_fort",
--     skin = "code_coffre_fort.png"
--   }
