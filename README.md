# LogConsole

**LogConsole** is a small, embeddable Qt widget for displaying, filtering and formatting application log output in a compact GUI console. It is designed to be easy to integrate into Qt-based applications and provides quick access to runtime messages (debug, info, warning, critical) with convenient filtering, coloring and basic performance optimizations for large logs.


## Key features

* Quick drop-in `LogConsoleWidget` for Qt (Widgets).
* Supports and displays Qt message types: Debug, Info, Warning, Critical.
* Toggle which columns are shown (Date, Time, Level, Source function, Message).
* Per-component / per-function filtering (can hide logs from specific functions such as `main`).
* Customizable colors for date/time, level, source function and message text/background.
* Font customization (change font family and size).
* Fast enough to handle large log files (rough benchmark: loading ~10,000 lines ~5 s; sorting dependent on settings, typically <5 s).
* Optional file logging and configurable text encoding helper utilities (see `LoggingEncoder`).
* Can be built as a library and embedded into other projects.


## Quick start

Include the header and create a console with one line:

```c++
#include <QApplication>
#include "Logging.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    using namespace Logging;
    LogConsoleWidget* console = quickNewConsole();
    console->show();

    qDebug() << "debug string";
    qWarning() << "warning string";
    qInfo() << "info string";
    qCritical() << "critical string";

    return a.exec();
}
```

This will create a ready-to-use LogConsole window and route Qt logging output to it.


## Public API (high level)

* `Logging::quickNewConsole(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())` – create and return a new `LogConsoleWidget` instance and automatically install the message handler.
* `Logging::setEnableFileLogging(bool enable)` – enable/disable logging to file.
* `Logging::setEnableConsoleLogging(bool enable)` – enable/disable routing Qt messages to the console.
* `Logging::setEnableDebug(bool enable)` – enable/disable debug-level messages.
* `Logging::setEnableFileEncoding(bool enable)` – enable/disable file encoding helper (see `LoggingEncoder`).
* `Logging::setLogConsole(LogConsoleWidget *Console)` / `Logging::getLogConsole()` – set or retrieve the currently used console instance.

Refer to the headers (`Logging.h`, `LogConsoleWidget.h`) for the full API and additional helpers.



## Building

The project is intentionally structured to support **two different workflows**:

1. **Development of the module itself** using **qmake (`.pro`)**.
2. **Integration into other projects** as a **library** using **CMake** or **qmake (`.pri`)**.

This allows you to comfortably develop and test LogConsole as a standalone module, while still embedding it cleanly into larger projects.

---

### 1. Module development (qmake / `.pro`)

For developing and debugging LogConsole itself, the primary entry point is the provided **`.pro`** file.

```bash
# from project root
qmake LogConsole.pro
make
```

This workflow is recommended when:

* You are modifying LogConsole source code.
* You want to run or debug the included example.
* You prefer Qt Creator with qmake-based projects.

You can open `LogConsole.pro` directly in **Qt Creator** and work on the module as a regular Qt Widgets application/library.

---

### 2. Using LogConsole as a library (CMake)

LogConsole can be linked into an existing **CMake-based** project as a library.

Basic out-of-source build:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

In your own `CMakeLists.txt`, you can then link against the exported target (assuming LogConsole is added as a subdirectory):	

```cmake
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/LogConsole)
target_link_libraries(${PROJECT_NAME} PUBLIC LogConsoleLibrary)
```

This approach is recommended when:

* Your main project already uses CMake.
* You want clean dependency management and target-based linking.
* You plan to use LogConsole as a reusable internal or external module.

---

### 3. Using LogConsole as a library (qmake / `.pri`)

For **qmake-based projects**, LogConsole can also be integrated via a **`.pri`** file.

Typical usage in your project `.pro` file:

```qmake
# include($$PWD/LogConsole/LogConsole.pri)
# INCLUDEPATH += $$PWD/LogConsole
```

This will add the required include paths, sources, and Qt modules to your project automatically.

This approach is useful when:

* Your main project is based on qmake.
* You want simple source-level inclusion without maintaining a separate build.
* You want consistency with existing `.pro`-based workflows.

---

### Summary

| Goal                       | Recommended method |
| -------------------------- | ------------------ |
| Develop LogConsole itself  | `.pro` (qmake)     |
| Embed into a CMake project | `CMakeLists.txt`   |
| Embed into a qmake project | `.pri`             |

This hybrid setup is intentional and allows LogConsole to be used flexibly across different Qt build systems.




## Integration tips

* The console installs a Qt message handler so regular `qDebug()`, `qInfo()`, `qWarning()` and `qCritical()` calls are displayed automatically.
* For long-running applications that generate many log lines, consider filtering or disabling `Debug` messages in production builds to reduce GUI overhead.
* The widget exposes settings for color and font; you can persist and restore them if you want consistent look-and-feel between runs.


## Resources

Project resources (icons and stylesheet) are included in the `resources/` directory and are registered via the resource file `ConsoleResources.qrc`.



## Performance note

Benchmarks indicate approximate processing speeds (results will vary depending on hardware and Qt version):
* Loading ~10,000 lines: ~5 seconds.
* Sorting: usually under 5 seconds depending on active columns and filters.



## Tests & Examples
* A minimal `main.cpp` example is included which demonstrates creating the console and emitting sample messages.

## License

This project is licensed under the MIT License — see the `LICENSE` file for details.


## Contributing

Contributions, bug reports and feature requests are welcome. Please open an issue or a pull request on GitHub. If you intend to submit a PR, please include a short description of the change and a minimal test case when appropriate.


