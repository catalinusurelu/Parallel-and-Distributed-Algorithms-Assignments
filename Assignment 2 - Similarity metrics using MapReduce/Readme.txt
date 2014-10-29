Usurelu Catalin Constantin
333CA

Tema 2 - Map Reduce

Observatie ! Am inclus si proiectul din Eclipse, si sursele separat, nu am inteles
exact cum trebuie trimis (mi s-a parut ciudat sa trimit doar sursele, ar fi mai 
greu de compilat, cred ...). Sursele sunt identice, nu am pus intr-o parte ceva
si in alta altceva.

Pentru a fi mai usor de implementat am folosit workpool-uri separate pentru
taskurile de map, respectiv reduce. Fiecare workpool are o clasa specifica,
MapWorkpool, ReduceWorkpool, acestea diferind intre ele doar prin tipul solu-
tiei partiale cu care lucreaza (ReducePartialSolutin, MapPartialSolution).

Partea de map:

Citirea o fac in paralele in fiecare MapWorker. Fiecare MapWorker primeste ca
solutie partiala pe care lucreaza (input) - numele fisierului, start-ul fragmentului
si sfarsitul fragmentului. Cand termina de numarat cuvintele, salveaza rezultatul
intr-un HashMap global din main, corespunzator numelui fisierului. HashMap-ul
are drept chei cuvinte si valori numarul de aparitii.
Pentru a detecta daca incep in mijlocul unui cuvant, citesc si caracterul de dinainte,
ca sa imi dau seama daca acest cuvant este nou (are inainte un delimitator) sau
e mijlocul unui cuvant (are inainte o litera). La sfarsit adaug caractere la fragmentul
obtinut pana cand intalnesc un delimitator.

Partea de reduce:

Un worker reduce are ca input (solutie partiala), numele fisierului cu care facem compa-
ratia (ca cel pentru care verificam daca e plagiat, il stim), si 2 HashMap-uri corespun-
zatoare cate unui fisier (documentul pentru care verificam daca e plagiat si un altul);
acestea au fost obtinute dupa pasul de map; rezultatul obtinut il retinem intr-un array global
cu perechi (nume_fisier, similaritate). Pe acesta il sortez si afisez doar perechile care
au gradul de similaritate mai mare decat cel specificat.

