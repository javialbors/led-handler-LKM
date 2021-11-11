# LED Handler Linux Kernel Module

## Introducción

En esta práctica se implementa un Linux Kernel Model (LKM) que controla dos LEDs mediante cuatro botones conectados a los GPIOs de una Raspberry Pi Zero WH.

### ¿Qué es un LKM?

Un Linux Kernel Module es un código compilable y ejecutable que permite ser cargado y descargado en el kernel de un dispositivo de manera que éste ejecute funcionalidades en segundo plano sobre el sistema como por ejemplo, funcionalidades de gestión de drivers para la comunicación de hardware con el sistema operativo de la máquina.

En el caso de esta práctica, se creará un LKM en C que se encargará de gestionar la comunicación del sistema operativo con dispositivos hardware externos (dos LEDs y cuatro botones) para que gestione las interacciones (o interrupciones) con este hardware.

### ¿Qué son los GPIOs?

Un GPIO (del inglés General Purpose Input Output) es un pin de entrada y/o salida programables. Esto es útil para controlar el comportamiento de hardware externo de manera programática.

La Raspberry Pi Zero WH utilizada, dispone de 40 pines, de los cuales una gran mayoría se usan como GPIOs. En el caso de esta práctica, se han conectado cuatro botones y dos LEDs a seis GPIOs diferentes con el propósito de encender o apagar los LEDs mediante las pulsaciones de los diferentes botones, gestionando estas funcionalidades de manera programática.

## Implementación

A continuación se explican los pasos utilizados para la realización de esta práctica.

### Requerimientos

En primer lugar, deberemos instalar los kernel headers correspondientes a nuestra versión del kernel en el caso que no estén instalados. Los kernel headers son un conjunto de ficheros donde están descritas las funciones que utiliza el kernel, de manera que instalándolos podemos tener a nuestra disposición las funcionalidades necesarias para la implementación de módulos del kernel. Las funciones utilizadas en nuestro código se compilarán junto con nuestro código de manera que no se tenga que compilar todo el kernel.

Para saber qué versión de kernel tiene un dispositivo, podemos ejecutar el siguiente comando `uname -r`. En mi caso, la versión del kernel de mi dispositivo es la `5.10.63+`. Ejecutando el siguiente comando, podemos descargar los headers correspondientes a esta versión en el sistema Raspbian.

`sudo apt install raspberrypi-kernel-headers`.

**Nota:** Para otras versiones del kernel de Raspbian como por ejemplo la `5.10.17+`, es útil seguir los pasos del repositorio https://github.com/RPi-Distro/rpi-source para poder instalar los headers.

### Programación del LKM

Se pueden diferenciar dos tareas diferentes para la implementación de este LKM.

#### Estructura de un LKM

Un LKM se puede cargar en el kernel y descargar del kernel. Cuando se carga o descarga el módulo del kernel, se ejecutan las funciones `module_init()` y `module_exit()` de nuestro código, respectivamente. Cada una de estas dos funciones toman como parámetro una función que debemos implementar y será la que se ejecute con la ejecución `module_ini(<función_init>)` al cargar el módulo al kernel o `module_exit(<función_exit>)` al descargarlo del kernel.

Generalmente, la función `module_init()` se encarga de inicializar y/o pedir memoria para los recursos necesarios para el funcionamiento del LKM, como por ejemplo, la asignación de GPIOs y de interruption handlers, entre otras cosas.

Por el contrario, la función `module_exit()` se encarga de liberar la memoria utilizada por el LKM, y de desasignar los GPIOs utilizados y interruption handlers.

#### Control de los GPIOs

Para controlar los GPIOs de los LEDs y botones de la práctica, debemos relacionarlos en el código. Para esto, asignaremos en variables estáticas globales el número de GPIO utilizado para cada LED o botón utilizado. Por ejemplo, `static unsigned int gpio_led1 = 16;` significa que relacionamos en el código uno de los LEDs con el GPIO número 16 ya que estará conectado físicamente a este GPIO.

Después de relacionar cada dispositivo con su respectivo GPIO en el código, se debe de pedir memoria para estos GPIOs en la función que será ejecutada por `module_init()`. Esto se realiza mediante la función `gpio_request()`. Por ejemplo, `gpio_request(gpio_led1, "sysfs")` donde 'sysfs' es un sistema de archivos virtual del kernel que gestiona información sobre los GPIOs y dispositivos externos.

A continuación se debe indicar la direccionalidad que tendrá cada GPIO. Por ejemplo, en el caso de un LED, la direccionalidad será de salida mientras que la de un botón será de entrada. Por ejemplo, `gpio_direction_output(gpio_led1, 0)` para un LED (donde `0` es el valor por defecto) o `gpio_direction_input(gpio_bt1)` para un botón.

En el caso de los botones, es interesante añadir la función `gpio_set_debounce()` donde podemos añadir un tiempo en milisegundos encargado del control de rebotes del botón.

Para activar los GPIOs configurados, se debe ejecutar la función `gpio_export()` para cada GPIO asignado. Por ejemplo `gpio_export(gpio_led1)`. Cuando el módulo se cargue en el kernel, se podrán observar estos GPIOs activos ya que se creará un directorio identificativo para cada uno de éstos en `/sys/class/gpio/`.

Para detectar una pulsación en un botón, se asignan unas interrupciones llamadas interruption requests a cada GPIO de cada botón y se guarda un valor entero de la interruption request asignada para utilizarlo más adelante. Una interruption request es una señal (o signal) que mandará a la CPU de nuestro dispositivo cada botón al ser pulsado para que la CPU pare de ejecutar otros procesos y gestione esta interrupción de manera adecuada, ejecutando un interruption handler, función que se ha implementado y que se explicará más adelante, que da respuesta a la interruption request (en este caso, el interruption handler encenderá o apagará el LED, entre otras cosas).

Para asignar la interruption request a un botón se utiliza la función `gpio_to_irq()`. Por ejemplo `irq_bt1 = gpio_to_irq(gpio_bt1)`.

Por último, se asignan las interruption requests a su interruption handler respectivo mediante la función `request_irq()`. Por ejemplo, `request_irq(irq_bt1, (irq_handler_t) bt1_handler, IRQF_TRIGGER_RISING, "bt1_handler", NULL);` donde `irq_bt1` es la interruption request que hemos definido previamente y `bt1_handler` será la función de tipo `irq_handler_t` que se ejecutará cuando se presione el botón número uno en el caso de este ejemplo.

Siguiendo estos pasos, ya estarían en memoria los GPIOs que se utilicen con sus respectivas configuraciones además de los interruption handlers para cada botón. Hay que recordar que estos pasos se han implementado dentro de la función que será ejecutada al ejecutar `modules_init()`.

Después del bloque de inicialización, se implementan los interruption handlers de cada botón. Éstos siguen la estructura que se explicará a continuación.

Primeramente se asignará el estado del LED a 1 o a 0 dependiendo de la funcionalidad del botón que se haya presionado mediante la función `gpio_set_value()` y utlizando el GPIO del LED respectivo.

Por último, gracias a la función `call_usermodehelper()`, se ejecutará un script externo, diferente para cada botón, que lo único que hará en esta fase de la práctica será crear un fichero para cada botón pulsado.

Después de crear las funcionalidades que gestionan las interruption requests de los botones, únicamente falta por explicar el contenido de la función que ejecuta la función `module_exit()`.

Esta función, como se ha explicado previamente, se encarga de liberar memoria como la utilizada para los GPIOs y interruption handlers, ya que se ejecutará al descargar el módulo del kernel y será memoria que ya no se va a utilizar.

Esta función primeramente apagará los LEDs, asignando a 0 su valor mediante la función `gpio_set_value()`. A continuación se desasignarán los GPIO utilizados en el LKM por los botones y LEDs mediante la función `gpio_unexport()` y se liberará la memoria reservada para los GPIOs utilizando la función `gpio_free()`.

Por último, se liberará la memoria utilizada para la gestión de interruption requests mediante la función `gpio_free()`.

De esta manera se han implementado las funcionalidades de los tres bloques necesarios para la realización de esta práctica. La inicialización del módulo, la gestión de interruption requests y la salida o descarga del módulo del kernel.

Cabe destacar que para cada funcionalidad del LKM, se forman mensajes de log utilizando la función `printk()` de manera que estos se pueden ver reflejados en el fichero `/var/log/kern.log`. En este fichero podrá ver reflejada la actividad del LKM en todo momento, como por ejemplo, ver el estado del LED tras una pulsación, ver cuantas veces se ha pulsado un botón, etc.

### Instalación del LKM

Una vez implementado el archivo en C, sólo queda compilarlo e insertarlo en el kernel de nuestro dispositivo.

Para la compilación, he creado un fichero Makefile que contiene el comando para la compilación el cual es el siguiente:

`make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules`

Como se puede observar, se compila el LKM mediante los headers de la versión del kernel del dispositivo que se han instalado previamente.

Una vez el código haya compilado correctamente, se puede observar que se crean múltiples archivos nuevos con el mismo nombre que nuestro archivo `.c` pero con diferentes extensiones.

Se podrá observar un archivo con extensión `.ko` la cual es una extensión que se asocia con los archivos de formato LKM. Éste será el archivo que deberemos cargar en el kernel mediante el siguiente comando:

`sudo insmod <lkm>.ko`

Una vez insertado satisfactoriamente el módulo en el kernel, podemos observar los mensajes de log de inicialización o de interacción con los botones viendo los mensajes del sistema (por ejemplo, ejecutando `dmesg | tail -5`) o el fichero de log del kernel con el comando `cat /var/log/kern.log`.

Adicionalmente, ejecutando el comando `lsmod` se listarán todos los módulos cargados actualmente en el sistema, donde se podrá comprobar que el módulo que hemos cargado está presente.

Para descargar o sacar el módulo del kernel de nuestro dispositivo se deberá ejecutar el siguiente comando:

`sudo rmmod <lkm>.ko`

Nuevamente, se podrán observar nuevos mensajes de log en el archivo `/var/log/kern.log` correspondientes a la salida del LKM del kernel.

#### Makefile

Para que la gestión del LKM en términos de instalación/desinstalación/limpieza de archivos sea más fácil, he creado un script de instalación Makefile completo con diversos modos de ejecución.

Los modos de ejecución de este Makefile son los siguientes:

* **setup** - Este modo da permisos de ejecución a los scripts externos que ejecuta cada botón al ser pulsado.
* **install** - Este modo compila el archivo C que contiene el LKM y lo inserta en el kernel.
* **clean** - Este modo descarga el módulo del kernel y borra los archivos creados por la instalación y los archivos extra creados por los botones.
* **restart** - Este modo ejecuta el modo **clean** y, seguidamente, el modo **install**.

Para ejecutar uno de los modos, se debe ejecutar el comando `make` seguido del modo a ejecutar. Por ejemplo, `make install` para compilar e insertar el módulo en el kernel.

## Bibliografía

* https://github.com/RPi-Distro/rpi-source
* https://www.raspberrypi.com/documentation/computers/linux_kernel.html
* http://derekmolloy.ie/writing-a-linux-kernel-module-part-1-introduction/
* http://derekmolloy.ie/kernel-gpio-programming-buttons-and-leds/
* https://developer.ibm.com/articles/l-user-space-apps/
