# ============================
# Makefile pour le Serveur TCP
# ============================

# 🎯 Variables
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I.
TARGET = serveur
SRC = main.c
OBJ = $(SRC:.c=.o)

# 🛠️ Règle par défaut
all: $(TARGET)

# 🎯 Compilation du programme principal
$(TARGET): $(OBJ)
	@echo "🔧 Compilation du serveur..."
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)
	@echo "✅ Compilation terminée."

# 🎯 Compilation des fichiers sources en objets
%.o: %.c
	@echo "🔨 Compilation de $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# 🧹 Nettoyage des fichiers objets et exécutables
clean:
	@echo "🧹 Nettoyage des fichiers objets..."
	rm -f $(OBJ)
	@echo "🧹 Nettoyage terminé."

fclean: clean
	@echo "🧹 Suppression de l'exécutable..."
	rm -f $(TARGET)
	@echo "🧹 Suppression terminée."

# 🔄 Recompilation complète
re: fclean all

# 📝 Aide
help:
	@echo "📝 Commandes disponibles:"
	@
