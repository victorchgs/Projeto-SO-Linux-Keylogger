#include "find_event_file.h"

// Diretorio onde os arquivos de evento estão guardados
#define INPUT_DIR "/dev/input/"

// Funcao que descobre se o arquivo passado eh um dispositivo de caracteres
static int is_char_device(const struct dirent *file)
{
  struct stat filestat;
  char filename[512];
  int err;

  // Funcao que concatena INPUT_DIR com o nome do arquivo passado
  snprintf(filename, sizeof(filename), "%s%s", INPUT_DIR, file->d_name);

  // Funcao que verifica se o arquivo eh um dispositivo de caracteres
  err = stat(filename, &filestat);
  if (err)
  {
    return 0;
  }

  return S_ISCHR(filestat.st_mode);
}

char *get_keyboard_event_file(void)
{
  char *keyboard_file = NULL;
  int num, i;
  struct dirent **event_files;
  char filename[512];

  // Percorre o diretorio e retorna o numero de arquivos no diretorio (filtrados)
  num = scandir(INPUT_DIR, &event_files, &is_char_device, &alphasort);
  if (num < 0)
  {
    return NULL;
  }
  else
  {
    for (i = 0; i < num; ++i)
    {
      int32_t event_bitmap = 0;
      int fd;
      int32_t kbd_bitmap = KEY_A | KEY_B | KEY_C | KEY_Z;

      snprintf(filename, sizeof(filename), "%s%s", INPUT_DIR, event_files[i]->d_name); // Concatenacao
      fd = open(filename, O_RDONLY);                                                   // Abre o arquivo

      if (fd == -1)
      {
        perror("open");
        continue;
      }

      // Verifica os tipos de eventos suportados pelo arquivo
      ioctl(fd, EVIOCGBIT(0, sizeof(event_bitmap)), &event_bitmap);
      if ((EV_KEY & event_bitmap) == EV_KEY) // Se os eventos forem eventos de teclado
      {
        // Verifica os eventos do tipo EV_KEY (Especificamente de teclado)
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(event_bitmap)), &event_bitmap);
        if ((kbd_bitmap & event_bitmap) == kbd_bitmap) // Se os eventos forem compativeis com kbd_bitmap
        {
          keyboard_file = strdup(filename); // Copia o diretorio do arquivo para keyboard_file
          close(fd);
          break;
        }
      }

      close(fd);
    }
  }

  // Limpa o array event_files, liberando a memoria
  for (i = 0; i < num; ++i)
  {
    free(event_files[i]);
  }

  free(event_files);

  return keyboard_file;
}
