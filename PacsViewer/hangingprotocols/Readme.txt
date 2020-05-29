Explicaci� estructura directori hanging protocols
-------------------------------------------------

/hangingprotocols
   |
    -- Default          // En aquest directori hi va tot el que anir� a l'instal�lador per defecte
   |     |
   |     |
   |      -- Specific   // Aqu� aniran HP que s�n espec�fics d'algun lloc en concret per� que no "fan mal" a la resta de llocs
   |     |     |
   |     |     |
   |     |      -- Site1
   |     |     |
   |     |     |
   |     |      -- Site1
   |     |     |
   |     |     |
   |     |      -- ...
   |     |
   |     |
   |      -- CT         //
   |     |
   |     |
   |      -- ...
   |
    -- Specific         // Directori on aniran els HP espec�fics d'un lloc organitzats per lloc
   |     |              // A difer�ncia de l'Specific de Default, aquests no s'instal�laran per defecte a tot arreu
   |     |              // i els haur� d'instal�lar a part l'administrador. Tamb� hi poden anar HP que els usuaris demanin personalitzats
   |     |
   |      -- Site1
   |     |     |
   |     |     |
   |     |      -- User1
   |     |
   |      -- Site2
   |     |
   |     |
   |      -- ...
   |
   |
    -- Testing          // Directori on aniran els HP de proves, test dels mateixos HP, etc.
