CC = gcc
CFLAGS = -g
OBJDIR = objs

# Список исходных файлов
SRC = main.c region_detector.c colorizer.c bmp_handler.c graph.c utils.c
# Добавил bmp_random_lines.c из вашего первого скриншота, если он используется
# SRC += bmp_random_lines.c


# Преобразуем список исходных файлов в список объектных файлов с префиксом OBJDIR
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

# Название исполняемого файла
TARGET = CourseWork

# Основная цель: создание исполняемого файла
# Здесь мы убираем $(OBJDIR) из списка зависимостей для ЛИНКОВКИ,
# но сохраняем его как зависимость для создания самого OBJDIR.
$(TARGET): $(OBJ) | $(OBJDIR) # Используем | для order-only prerequisite
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# Правило для создания директории OBJDIR
# Это правило будет выполняться только один раз, если OBJDIR не существует
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Правило для компиляции .c файлов в .o файлы внутри OBJDIR
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Цель для очистки
clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)

.PHONY: all clean