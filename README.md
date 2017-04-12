# Blowfish
Blowfish chiper using POSIX multithreading

1 Salva tutti i file nella stessa cartella
2 Apri un terminale in quella cartella
3 Digita il comando make clean 
4 Digita il comando make ( per compilare tutti i file)
5 Verrà generato un file eseguibile di blowfish
6 Per eseguite blowfish il comando è :         ./Blowfish key inputfile c/d outputfile
  
Ad esempio, se vuoi cifrare il file ciao.txt con la chiave abcdefghijklmno e salvarlo come ciao2.txt devi digitare:
        
                         ./Blowfish abcdefghijklmno ciao.txt c ciao2.txt

per decifrarlo basta invertire:

                         ./Blowfish abcdefghijklmno ciao2.txt d ciao3.txt

n.b. : Per cifrare o decifrare un file devi metterlo nella stessa cartella in cui hai salvato i file di Blowfish
