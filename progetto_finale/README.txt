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

--- NOTE ---

Per problemi di tempo non siamo risuciti a suddividere in maniera più ordinata i vari file.
La nostra idea originale era quella di creare anche un TimeManager e un ConsumptionManager per alleggerire file come DeviceManager.cpp.
Sempre per problemi di tempo nel file CommandManager.h è presente anche l'implemnetazione dei metodi e non solo la dichiarazione, avremo dvouto separare le due cose.

Il file CMakeLists.txt permette di compilare sia su macos (testato su Sequoia 15.1.1) che Linux (testato su Ubuntu 22.04 ARM64).

Il progetto utilizza solo la libreria standard ed è scritto intermanete in c++11.

Ci sono alcune implementazioi possibili a cui avevamo pensato ma che non erano richieste dal progetto come ad esempio la gestione del timer solo di accensione.
Se io infatti scrivo ad esempio 'set <deviceName(che sia manuale o a spegnimento automatico)> on HH:MM HH:MM' lo accenderà al primo orario e lo spegnerà al secondo 
(a meno che non si tratti di un dispsoitvo a spegnimento automatico e il tempo che passa tra i due orari, di accensione e spegnimento, sia maggiore della durata del ciclo, in quel caso si spgenrà al termine del suo ciclo). 
Se invece scrivo 'set <deviceName> on HH:MM' , attualemnte non viene gestito, prende comunque il comando per buono ma in relatà accende il dispositivo all'orario attuale.
