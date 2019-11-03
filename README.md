# Progetto_Msgbox_SO-18-19
Progetto di Sistemi Operativi per l'A.A. 2018-2019
Francesca Fiani

Msgbox module

Il modulo è formato da tre parti:

    SharedFunctions: Contiene le funzioni che gestiscono la msgbox (creazione, lettura e scrittura sia della shmem che dei topic).
    SharedReader: Main che si occupa del funzionamento dei lettori della msgbox.
    SharedWriter: Main che si occupa del funzionamento dello scrittore della msgbox.

La msgbox funziona tramite una coppia di semafori e permette di impiegare molteplici lettori simultanei e un singolo scrittore.
Il writer può creare molteplici topic e passare da uno ad un altro a proprio piacimento tramite il comando 'exit'.
Il reader invece può connettersi ad un singolo topic, il cui nome va passato in argv, per tutta la sua vita.

Si noti che ciascun lettore/scrittore occupa un terminale e che possono essere terminati tramite CTRL+C per assicurare la chiusura della msgbox corrente. Per avere una chiusura ottimale, si consiglia di usare il comando 'quit' su writer.
