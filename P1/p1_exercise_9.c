/*
 * Ejemplo de codigo que genera un numero aleatorio y lo muestra por pantalla
 */
 #include <sys/types.h>
 #include <time.h>
 #include <stdlib.h>
 #include <stdio.h>

int main(int argc, char *argv[]) {
  /* Inicializa el generador con una semilla cualquiera, OJO! este metodo solo
   se llama una vez */
  srand(time(NULL));
  /* Devuelve un numero aleatorio en 0 y MAX_RAND(un número alto que varia
   segun el sistema) */
  int r = rand();
  printf("%d\n", r);

  exit(EXIT_SUCCESS);
}
