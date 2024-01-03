## Acerca del Proyecto Alice.

### Para empezar:
Por Favor consulte docs/contributing.md para más información! Contiene información de como compilar el proyecto en Windows y Linux

### Continuando desde donde se quedó OpenV2:

El objetivo del Proyecto Alice (nombrado a partir de [Alice Roosevelt Longworth](https://en.wikipedia.org/wiki/Alice_Roosevelt_Longworth)) es esencialmente crear una nueva version de [open v2](https://github.com/schombert/Open-V2) -- mi proyecto anterior para crear un clon de Victoria 2 -- y luego terminar con una version funcional y completa del juego. Esto significa que, al menos inicialmente, habrá pocas desviaciones en la forma en que se hacían las cosas en Victoria 2, simplemente para mantener el proyecto enfocado y en el camino correcto. Una vez que se complete la versión 1.0, podremos usarla como un campo de pruebas para nuevos experimentos. Hasta julio de 2023, básicamente he alcanzado donde estaba Open V2, excepto algunas deficiencias en la interfaz de usuario.

Ahora mismo hemos hecho un progreso significativo para reconstruir open v2, la mayor parte hecha desde cero, pero no nos hemos acercado al nivel de donde open v2 estaba. En cambio, el trabajo ha ido más rápido que lo que se hizo la primera vez, parcialmente porque la gente que ahora contribuye lo hace con sus propios talentos (en comparación de open v2 que era de una sola persona), y parcialmente porque no tendré que hacer los errores que cometí la primera vez.

Los programadores que trabajan en el proyecto se dividen en UI, el Mapa, el Núcleo y "Freelancer" (posiblemente añadiendo al Launcher). UI y Mapas se enfocan, como su nombre implica, en la interfaz de usuario y el mapa, respectivamente. El núcleo solamente por ahora soy yo, y colecta programadores enfocados en implementar la lógica interna del juego. Finalmente, Freelancer es para todos los demás que han hecho contribuciones pero no se han decidido en un grupo en particular. ¿Cómo te puedes unir a uno de esos grupos? Solamente haz una PR que sea aceptada en el proyecto ([Contributing](contributing.md) para más detalles).

Espero encontrar algunas personas para formar un equipo de arte que pueda recrear los activos que Victoria 2 utiliza para darle a este proyecto su propia identidad visual distintiva. Sé que es una gran solicitud, pero, por otro lado, no se requerirá modelado 3D. Ya tenemos acceso a un conjunto de nuevas banderas que podemos usar, cortesía del proyecto SOE.

### Ética de trabajo:

Personalmente, estoy dispuesto a trabajar en este proyecto al menos 1 o 2 horas cada día de trabajo. Esto no se refleja necesariamente en el tiempo programado. Puede ser tiempo invertido en documentar o investigar, o solamente de idear. Pero para mí, pienso que mi disposición de hacer algo con persistencia es importante.

### En comparación con otros proyectos:

En comparación con SOE (Symphony of Empires/ Sinfonía de Imperios), este proyecto es un clon más directo de Victoria 2, mientras que SOE es su propio juego. Sin embargo, tiendo a pensar en él como un proyecto hermano. Tenemos algunas personas aquí involucradas en ambos (yo mismo hago sugerencias de vez en cuando allí), y si ves algo que hagamos que ellos puedan usar, estoy seguro de que apreciarían la contribución. De manera similar, usaremos al menos parte de su trabajo.

En cuanto al proyecto OpenVic2, aunque se "comercializa" como un "clon" de Victoria 2, no es hacia eso hacia donde están trabajando. En cambio, tienen la intención de proporcionar un conjunto completamente nuevo de activos, incluyendo eventos, decisiones, etc., lo que hará que su proyecto sea más parecido a Victoria 2. También se ejecuta con un estilo de gestión muy diferente. Si estás interesado en ambos Project Alice y OpenVic2, sugiero pasar tiempo con ambos equipos o incluso unirte a ambos proyectos.

### Licencia:

Todo el código producido como parte de este proyecto se lanzará bajo la licencia GPL3 (como lo fue Open V2). La licencia para los activos artísticos dependerá del equipo de arte, pero preferiría alguna forma de Creative Commons.

### Herramientas de programación:

El proyecto utiliza CMake (sí, sé que todos odiamos CMake). Idealmente, debería poder compilarse con cualquiera de los tres grandes compiladores: MSVC, Clang/LLVM o GCC. Personalmente, tiendo a usar MSVC, pero también uso Clang en Windows. Asegurarse de que las cosas sigan funcionando en GCC será problema de otra persona, y si ningún colaborador resulta ser un usuario regular de GCC, la compatibilidad con GCC podría desaparecer de esta lista.

Personalmente, uso Visual Studio 2022 (la edición gratuita, community) para trabajar en el proyecto, pero si estás contribuyendo, deberías poder usar las herramientas que prefieras, incluyendo VS Code, CLion, EMACS, VI, etc.

### ¿Dónde se apartará el Proyecto Alice de Victoria 2?

Inicialmente, Project Alice imitará mecánicamente a Victoria 2 lo más cercanamente posible, con algunas excepciones menores. Sí, hay muchas cosas que podríamos mejorar. Entré en Open V2 con la mentalidad de que simplemente mejoraríamos algunas cosas aquí y allá a medida que avanzábamos, y esa fue una de las razones principales por las que el proyecto se desvió y, en última instancia, se abandonó. No quiero cometer ese error nuevamente, así que esta vez voy a intentar resistirme a la tentación de hacer mejoras hasta que tengamos un juego funcional y completo. Dicho esto, hay algunos cambios que debemos hacer de todos modos. El mapa del globo es simplemente demasiado divertido como para dejarlo fuera. Y hay algunas mejoras de calidad de vida (como enlaces que te llevan a lo que un evento está hablando o un resumen de por qué se desencadenó un evento) que son lo suficientemente fáciles y agradables como para ser una verdadera lástima dejarlas fuera. ¡Pero no habrá ajustes importantes en la mecánica esta vez! ¡Lo prometo!
