### Program Flow Diagram

Assuming the program does not terminate early (encountered error or ran with certain flags),
below is the flow diagram of the program.

```
╔════════╦════════╦══════════╦════════════════╦═════════════════════════════════════════╦═══════════╦═══════╗
║        ║        ║          ║                ║                                         ║           ║       ║
║ Start ─╫─> API ─╫─> Parse ─╫─> Lock/Unlock ─╫─> Integrate? ─n─> Run Shell Command ────╫─────────┬─╫─> End ║
║        ║        ║          ║                ║       │                                 ║         │ ║       ║
║        ║        ║          ║                ║       └─────y───> Fork? ─parent─> Wait ─╫─────────┤ ║       ║
║        ║        ║          ║                ║                     │                   ║         │ ║       ║
║      ┌─╫────────╫──────────╫─────child──────╫─────────────────────┘                   ║         │ ║       ║
║      │ ║        ║          ║                ║                                         ║         │ ║       ║
║      └─╫─> API ─╫─> Parse ─╫────────────────╫─────────────────────────────────────────╫─> Main ─┘ ║       ║
║        ║        ║          ║                ║                                         ║           ║       ║
║        ║        ║          ║                ║                                         ║           ║       ║
║        ║libfunc ║parse     ║interface       ║gtkfunc                                  ║userspace  ║       ║
╚════════╩════════╩══════════╩════════════════╩═════════════════════════════════════════╩═══════════╩═══════╝
```
