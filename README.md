# md5-calculator
Calculador de md5 de archivos

### Modulos

| Modulo | Descripcion |
| ------ | ------ |
| **App** | Realiza el instanciamiento de procesos hijo para procesar hashes, la creación del shared memory para conectarse con la vista y la escritura de los resultados en results.txt. |
| **Child** | Recibe los nombre de los files por entrada estandar y devuelve el resultado de md5sum al App |
| **View** | Se conecta al App mediante shared memory, y obtiene los resultados a imprimir por ese buffer compartido. Tiene una limitacion de 4096 bytes de lectura por tamaño de shared memory. Si View recibe por argumento el nombre del shared memory al cual va acceder. SI no lo recibe por argumento espera que le llegue por entrada estandar |

### Ejecución
Hay varias formas de ejecutar los procesos. Uno compila con 
> ./run.sh compile

Después hay 2 formas de correr.
1. Utilizar pipe. Esto puede ser corriendo
> ./run.sh run [archivos a correr]

&nbsp;&nbsp;o

> ./app [archivos a correr] | ./view

<br>
2. Utilizando dos terminales o corriendo app en background, uno puede correr

> ./app [archivos a correr]

&nbsp;&nbsp; obtener el nombre del shared memory y correr en otra terminal

> ./view [nombre de shared memory]

App espera 2 segundos (por consigna) antes de correr por su propia cuenta. El view no se puede conectar despues de esos 2 segundos.

<br>

### Limpieza
Para limpiar los archivos y objetos generados, utilizar ./run clean.