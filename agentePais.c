#include "COMUN.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

// Funcion para enviar un mensaje por el pipe
int enviarMensaje(int dp, Mensaje mensaje) {

    // Enviamos el mensaje
    ssize_t resultado = write(dp, &mensaje, sizeof(Mensaje));

    // Revisamos si hubo un error
    if (resultado != sizeof(Mensaje)) {
        perror("Error al enviar mensaje");
        return -1;
    }

    return 0;
}
