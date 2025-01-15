SistemaDomotico

--- INTRODUZIONE ---

Il seguente progetto è parte del corso di Laboratorio di Programmazione (progetto finale). È stato svolto dal gruppo "_sudo rm -rf /*" (underscore per motivi di sicurezza).
Questo progetto ha come obiettivo il simulare la gestione di elettrodomestici ed altri dispositivi facenti parte di una casa.

--- STRUTTURA DEL PROGETTO ---

La seguente struttura del progetto è suddivisa in più file per questioni di modularità e semplicità di gestione del codice:
SistemaDomotico
├── include
│     │── CommandManager.h
│     │── Device.h
│     └── DeviceManager.h
├── README.txt
|--CMakeList.txt
└── src
      ├── DeviceManager.cpp
      ├── device_implementation.cpp
      └── main_program.cpp

--- SUDDIVISIONE DEL LAVORO ---

Il progetto è stato sviluppato dai membri del gruppo, nel seguente modo:

Persona 1: Alessandro Farina

CommandManager.h: Creazione dell'header per la classe CommandManager, inclusi i metodi per eseguire i comandi ricevuti e la gestione delle funzioni di controllo.
main_program.cpp: Scrittura del programma principale, gestendo l'input dell'utente e l'interazione con il DeviceManager e il CommandManager.

Persona 2: Enrico Vanghetti

DeviceManager.cpp: Implementazione dei metodi definiti in DeviceManager.h per la gestione dei dispositivi e delle operazioni richieste (aggiunta dispositivi, gestione timer, ecc.).
main_program.cpp: Scrittura del programma principale, gestendo l'input dell'utente e l'interazione con il DeviceManager e il CommandManager.
Device.h: Creazione dell'header per la classe base Device che definisce le proprietà comuni di tutti i dispositivi, come il nome, l'ID e la potenza.

Persona 3: Edoardo Scudeller

device_implementation.cpp: Implementazione della logica dei dispositivi specifici, come i dispositivi manuali (ManualDevice) e a controllo automatico (FCDevice).
DeviceManager.h: Creazione dell'header per la classe DeviceManager, inclusi i metodi per aggiungere, rimuovere dispositivi e gestire il tempo.
README.txt: Scrittura del documento di introduzione e descrizione del progetto. Inclusione della struttura e suddivisione del lavoro.
