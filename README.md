# Farm – MasterWorker & Collector

**Corso:** Sistemi Operativi e Laboratorio (SOL), A.A. 22/23

**Autore:** Giuseppe Conte

`farm` è un programma in **C** che implementa un modello di calcolo distribuito tramite **processi e thread**, basato su un’architettura **MasterWorker & Collector**.
Il progetto gestisce file binari contenenti numeri interi lunghi, esegue calcoli sui dati e raccoglie i risultati in ordine crescente.

---

## Overview

* **MasterWorker** (multi-threaded):

  * 1 thread Master
  * `n` thread Worker (configurabili con `-n`)
* **Collector**: raccoglie i risultati dai Worker tramite **socket AF_LOCAL (AF_UNIX)**
* Il socket `farm.sck` viene creato nella directory del progetto e rimosso alla terminazione
* MasterWorker gestisce segnali: `SIGINT`, `SIGQUIT`, `SIGTERM`, `SIGHUP`, `SIGUSR1`
* Worker leggono file, calcolano:

```math
result = \sum_{i=0}^{N-1} i \cdot file[i]
```

e inviano `result + filename` al Collector

---

## Argomenti della riga di comando

```bash
-n <nthread>    # Numero di thread Worker (default 4)
-q <qlen>       # Lunghezza della coda concorrente Master-Worker (default 8)
-d <directory>  # Directory contenente file binari (opzionale)
-t <delay>      # Millisecondi tra invio di task ai Worker (default 0)
-h              # Mostra help/uso
[file1 file2 …] # Lista file binari da elaborare
```

> Se viene specificata `-d`, tutti i file nella directory e nelle sottodirectory vengono considerati input.

---

## Funzionamento

1. **MasterWorker** legge i file o la directory indicata.
2. Aggiunge i file nella **coda concorrente** dei task.
3. I thread **Worker** prelevano i task, leggono il file, eseguono il calcolo e inviano il risultato al **Collector** tramite socket.
4. Il **Collector** riceve tutti i risultati e li stampa ordinati in output:

```text
risultato1
risultato2
...
filepath1
filepath2
...
```

5. Ricezione di **SIGUSR1**: stampa parziale dei risultati ordinati senza terminare.
6. Ricezione di `SIGINT`, `SIGQUIT`, `SIGTERM`, `SIGHUP`: terminazione ordinata dopo completamento dei task in coda.

---

## Architettura del programma

* **MasterWorker**

  * Thread Master: legge file e produce task nella coda
  * Thread Worker: esegue calcolo sui file
  * Thread handler segnali
* **Collector**

  * Riceve risultati dai Worker tramite socket AF_LOCAL
  * Ordina e stampa i risultati
* **Coda concorrente**

  * Implementata come lista limitata di lunghezza `q` (bounded queue)
  * Gestione concorrenza tramite mutex e condition variables
* **Socket**

  * Collector = server
  * Worker = client
  * Supporta messaggi `close` e `sigusr1`

---

## Compilazione ed Esecuzione

### Compilazione

```bash
make        # Compila l'eseguibile farm
make debug  # Compila con flag DEBUG e print aggiuntive
make file   # Compila generafile per creare file binari di test
```

### Esecuzione

```bash
./farm -n 4 -q 8 -t 200 file1.dat file2.dat -d testdir
```

Oppure usare target test:

```bash
make test
make test2
```

### Pulizia

```bash
make clean
```

Rimuove oggetti `.o`, eseguibili e socket `farm.sck`.

---

## Esempio di output

```text
103453975
112546319
153259244
293718900
380867448
584164283
672594110
file2.dat
testdir/testdir/file7.dat
file1.dat
file3.dat
file5.dat
file4.dat
testdir/file6.dat
```

---

## Materiale incluso

* **File sorgente principali**

  * `main.c`, `masterworker.c`, `masterthread.c`, `poolworker.c`, `collector.c`, `myutil.h`
* **File di supporto dalle esercitazioni**

  * `boundedqueue.c`, `boundedqueue.h`, `util.h`, `conn.h`
* **Script di test**

  * `test.sh`
  * `generafile.c` / `generafile.sh` per creare file binari

---

## Note e limitazioni

* Alcune funzioni di sleep e gestione memoria sono sperimentali (debug con `make debug`)
* I file input devono avere estensione `.dat`
* Testato su Linux multi-core Ubuntu 21.04
