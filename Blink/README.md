Ce dossier contient les fonctions pour controller les leds

usb.c contient les fonctions envoyant la données dans l'endpoint du Louloupad

blink.c fait appel à usb.c (via usb.h) pour faire clignoter les led de la manette

Pour executer blink.c il suffit d'utiliser "make run" avec les droit administrateur !!
