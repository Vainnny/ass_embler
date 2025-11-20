CC = gcc
CFLAGS = -g

OBJDIR = objs

ifeq ($(OS),Windows_NT)
MKDIR_P = if not exist "$(OBJDIR)" mkdir "$(OBJDIR)"
RM_DIR = if exist "$(OBJDIR)" rmdir /s /q "$(OBJDIR)"
RM_FILE = if exist "$(TARGET)" del /f /q "$(TARGET)"
else
MKDIR_P = mkdir -p $(OBJDIR)
RM_DIR = rm -rf $(OBJDIR)
RM_FILE = rm -f $(TARGET)
endif

$(OBJDIR)/%.o: %.c
	$(MKDIR_P)
	$(CC) $(CFLAGS) -c $< -o $@

SRC = main.c region_detector.c colorizer.c bmp_handler.c graph.c utils.c
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))
TARGET = CourseWork

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM_DIR)
	$(RM_FILE)