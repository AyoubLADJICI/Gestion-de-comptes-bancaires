##📚 Projet Client-Serveur TCP/UDP - Application de gestion de comptes bancaire
###📝 Description du Projet
Ce projet est une application client-serveur permettant la gestion de comptes bancaires via les protocoles TCP et UDP. Les utilisateurs peuvent effectuer les opérations suivantes :

- AJOUT : Ajouter une somme d'argent à un compte.
- RETRAIT : Retirer une somme d'argent d'un compte.
- SOLDE : Consulter le solde actuel du compte.
- OPERATIONS : Afficher les 10 dernières opérations du compte.

L'application est divisée en deux versions :

TCP : Communication fiable et orientée connexion.
UDP : Communication rapide mais sans garantie de livraison.

##⚙️ Prérequis
- Système d'exploitation : Linux / Ubuntu ou bien un terminal Ubuntu si vous êtes sur Windows
- Compilateur : GCC

##🛠️ Instructions

- Etape 1 : Cloner le repository 
```bash
git clone https://github.com/AyoubLADJICI/Gestion-de-comptes-bancaires.git
cd Gestion-de-comptes-bancaires/
```

- Etape 2 : Se rendre sur le dossier souhaite
```bash
cd TCP/ 
```
ou
```bash
cd UDP/
```

- Etape 3 : Lancer la compilation
```bash
make
```
- Etape 4 : Executer le serveur
```bash
./server
```

- Etape 5 : Executer le client
```bash
./client 127.0.0.1 "votre_nom"
```
📝 Commandes Disponibles depuis le Client
Après la connexion, le client peut utiliser les commandes suivantes :

AJOUT <id_client> <id_compte> <password> <somme>
Exemple : AJOUT Ayoub26 compte2 password2 500

RETRAIT <id_client> <id_compte> <password> <somme>
Exemple : RETRAIT Ayoub26 compte2 password2 300

SOLDE <id_client> <id_compte> <password>
Exemple : SOLDE Ayoub26 compte2 password2

OPERATIONS <id_client> <id_compte> <password>
Exemple : OPERATIONS Ayoub26 compte2 password2

Voici les comptes disponibles dans cette banque :

id_client :Ayoub7
id_compte :compte1
password  :password1

id_client :Ayoub26
id_compte :compte2
password  :password2

id_client :Ayoub93
id_compte :compte3
password  :password3

