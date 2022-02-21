#tool
CC = gcc
#flags
CFLAGS = -std=gnu90

#directory dove si trovano i file da compilare
DIR = src


#ricetta help
#mostra brevi info testuali
#sinceramente non ho idea di cosa il professore voglia visuaizzare
help : 
    @echo "..."


#ricetta clean
#rimuove file temporanei e riporta tutto allo stato iniziale
clean :
    rm -f $(DIR)/*.o  build

#indica che clean non è un file da creare ma solo un comando
.PHONY: clean

#compilazione dei file
#da motare che non possono esserci degli "spazio" all'interno del nome
$(DIR)/%.o: *.c
    $(CC) $(CFLAGS) $< -o $@

#ricetta build
build : $(DIR)/*.o
    $(CC) $(CFLAGS) $< -o $@