# Ziggy80
Ziggy80 es una placa de hardware que emula un procesador Z80, con el propósito de reemplazar el procesador en sistemas hardware existentes, manteniendo su funcionalidad original y añadiendo capacidades adicionales.

Inicialmente desarrollado para computadoras MSX, específicamente para las computadoras Talent DPC200, Ziggy80 puede ser adaptado para otros sistemas basados en Z80, con ciertas restricciones:

* Actualmente, no implementa señales como RFSH, M1, RESET, BUSRQ, WAIT, BUSAK, HALT, NMI e INT. Se contempla la posibilidad de incorporarlas en futuras versiones. La señal WAIT en sistemas MSX se utiliza para agregar un T-state. Al estar implementada en el software podría generar incompatibilidades con hardware que realmente utilice esta línea, incluso en dispositivos MSX compatibles.
* El diseño está ajustado para replicar las señales de un procesador operando a 3.579 MHz, por lo que debe modificarse para funcionar a otras velocidades.
* Las líneas NMI e INT tampoco se tienen en cuenta actualmente. Para la interrupción generada por el VDP en MSX, se simula por software utilizando un timer interno del RP2040..

Estas restricciones buscan simplificar el diseño del hardware y software para emular únicamente el comportamiento esencial del Z80. Sin embargo, se considera la posibilidad de mejorar el proyecto en el futuro incorporando más características del Z80 original.

## Desafíos de Implementación
Uno de los desafíos clave fue la limitación de GPIOs. Se requerían al menos 29 líneas, pero la Raspberry Pi Pico solo dispone de 28 GPIOs utilizables. Se hicieron ajustes para adaptar el diseño a 16 GPIOs, optimizando la disposición y el manejo de estas líneas desde los PIOs y sus máquinas de estado.

Diagrama de bloques:
![Diagrama de bloques](/diagrama_de_bloques.svg)

La Raspberry Pi Pico funciona a 3.3V, lo que demandó el uso de adaptadores de niveles de tensión. Este diseño también ayudó a minimizar la cantidad de adaptadores a solo 2.

## Emulación
Para emular un Z80 se requirieron diversas partes:

* Un secuenciador para generar las señales IORQ, MREQ, WR y RD según la operación solicitada, limitado en la versión actual a lectura/escritura en memoria (RAM y ROM) y puertos de I/O.
* Para la emulación del comportamiento del Z80 por software, se estudiaron múltiples implementaciones de código abierto disponibles en la web y se optó por la versión de Marat Fayzullin.
* Los PIO y las máquinas de estado fueron fundamentales para sincronizar y respetar los tiempos de acceso. Su programación se realizó completamente en assembler, lo que requirió un aprendizaje adicional.

Accesos a memoria

![Diagrama de bloques](/memory_cycle_primary_slot.png)

Acceso a puertos I/O

![Diagrama de bloques](/IO_cycle_primary_slot.png)

De acuerdo a estos esquemas he diseñado el siguiente diagrama de flujos que tiene la particularidad de estar segmentado según el T-state donde debe ejecutarse cada parte:

![Diagrama de bloques](/IO_flow_chart.svg)

## Funcionamiento
Para el funcionamiento se definen 4 operaciones que deben estar presentes:

* Lectura de memoria
* Escritura de memoria
* Lectura de puerto  I/O
* Escritura de puerto I/O

Con este objetivo se crearon dos máquinas de estado con fines bien diferenciados. La primera es la que prepara todo para una de las 4 operaciones y sincronizar con ayuda de la segunda la secuencia de activación de las líneas de control involucradas en cada caso en sus respectivos ciclos T.

Entonces, la primer máquina se encarga de lo siguiente:

* Espera la ejecución de una operación y sus parámetros (dirección, datos a escribir o “0” si fuera el caso de una lectura y la configuración de dirección del bus)
* Se sincroniza con CPUCLK y dispara la ejecución en la segunda máquina que quedará generando paralelamente las señales MREQ, IORQ, WR y RD según sea la operación..
* Durante T1 en alto
  * Pone el bus de datos como salida
  * Presenta en el BUS la parte baja de ADDRESS y setea el 74LS574 ADDR LO
  * Presenta en el BUS la parte alta de ADDRESS y setea el 74LS574 ADDR HI
* Durante T1 en bajo	
  * Presenta los datos en el bus indistintamente si es una operación de escritura o lectura por una cuestión de simplificar el código. En caso de ser una lectura, solo se ponen ceros, pero no tiene importancia ya que el 74LS245 DATOS se encuentra a esta altura en alta impedancia manteniendo el BUS desconectado.
  * Configura la dirección del BUS como entrada o salida según sea el caso.
  * Espera indicación de la segunda máquina de estados que indicará cuando comience T3
* Durante T3 en alto
  * Independientemente si se trata de una lectura o una escritura, lee el dato en el BUS, también por simplificar el código. 
* Durante T3 en bajo
  * Envía el dato leído y termina la ejecución. Por fuera, el código que lanza la ejecución de esta máquina espera que la misma retorne este dato para continuar. Si el dato devuelto es de una operación de escritura, simplemente se descarta.

La segunda máquina se encarga como ya se mencionó anteriormente de la ejecución sincronizada de las señales IORQ, MREQ, RD y WR. También se encarga de la habilitación o deshabilitación en el momento indicado del 74LS245 usado para los datos.

Al principio de la ejecución, espera que se dispare una interrupción interna desde la máquina principal y a partir de esto empieza a generar las señales. Al llegar a T3 se lo indica a la primera máquina mediante otra interrupción, luego espera la finalización de T3 y termina. Tanto para operaciones de memoria como para I/O se ha insertado un wait state sin importar el caso para simplificar el código. Esto solo afecta en que toma un ciclo más de reloj aunque no sea necesario. 

En resumen, esta arquitectura de dos máquinas de estado interconectadas permite ejecutar de manera precisa y coordinada las operaciones esenciales, ofreciendo un control detallado sobre las señales y tiempos involucrados en el proceso.

## Conclusión
El proyecto Ziggy80 ha alcanzado un nivel de funcionalidad bien sincronizada que va más allá de simplemente reemplazar el procesador Z80. La implementación no solo ofrece una alternativa efectiva, sino que también proporciona una interfaz de programación accesible desde entornos como C/C++ y MicroPython, ampliando significativamente las posibilidades de desarrollo.

Pienso que lo más interesante de este proyecto es la diversidad de oportunidades que se despliegan. Desde herramientas de diagnóstico hasta la posibilidad de transformar la MSX en una consola moderna, Ziggy80 ofrece un lienzo para la innovación. La capacidad de ejecutar el Z80 a velocidades superiores, redirigir accesos a memoria y puertos I/O, característica que permite mejorar el hardware e incluso usar técnicas de sistemas operativos más avanzados, y la perspectiva teórica de tomar "instantáneas" del sistema antes de apagarlo, son solo algunas de las funciones que destacan la versatilidad del proyecto.

Además, la iniciativa de desarrollar ejemplos prácticos para demostrar estas capacidades añadirá valor y facilitará la adopción del proyecto por parte de la comunidad.

En este punto, quisiera extender una cálida invitación a otros apasionados por la tecnología a unirse a este objetivo. Con el potencial de colaboradores adicionales, podemos llevar Ziggy80 a nuevas alturas. Cada contribución, ya sea en forma de ideas innovadoras o mejoras prácticas, puede marcar la diferencia.

No olvidemos que también existe una versión de Raspberry PI Pico con capacidades de WIFI y Bluetooth, abriendo un abanico de posibilidades aún más amplio.

En conclusión, Ziggy80 no es solo una solución de “puro capricho”, sino una base para extender y transformar el sistema original MSX de maneras que aún no hemos imaginado por completo. 

## Agradecimientos
Desearía expresar mi sincero agradecimiento, en primer lugar, a Carlos Maidana, compañero clave en el desarrollo de este proyecto. Juntos, transformamos una idea inicial centrada en la creación de un tester de hardware para MSX en un proyecto integral y más abarcativo.

Asimismo, mi reconocimiento especial para Maximiliano López, futuro ingeniero electrónico, cuya comprensión profunda de la idea se reflejó de manera excepcional en el diseño del PCB.
