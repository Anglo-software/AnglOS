# Unnamed-OS

## Section 1 - Introduction

### 1.1 - What is this?

So you might be asking yourself, (or you might very well not) what is this thing?

Unnamed OS is an operating system built from the ground up, completely custom, with a main focus on being written primarily in Python ... That's right, Python!

"That's slow and horrible!" you might say.

Well, yeah, if I weren't also rewriting the Python interpreter, as well as writing a custom Python preprocessor, compiler, assembler, and linker, along with tweaking parts of the language itself to support such features as: static typing, proper memory management, and other fun things!

The kernel and very low level things will be written in C/C++ of course, but once an interpreter and compiler can be built on top of that, it's all Python, in theory. The extended version of Python should be backwards compatable with normal Python, and I'll try and keep it updated as often as possible.

At the moment, the baseline functionality is a text-based interface with a shell, file system, and necessary system tools.

[More will be added]

### 1.2 - Why?

For the meme

### 1.3 - When?

Sometime in the near future... on the cosmological scale.

### 1.4 - Who?

I'm just some bored college student who puts waaaaay too much effort into shitposting, who just so happens to have a keen interest in systems programming and low level stuff. Don't take any of this project seriously.

## Section 2 - Current Plan

### Section 2.1 - Big Picture

Kernel level:

Stage 1 (C/asm):
- Bootloading code with GRUB
- Bus driver module
- Network driver module
- Page allocator module
- Interrupts core module
- Block device module
- Dispatcher module
- HCI Peripheral module

Stage 2 (C++):
- System run and generic hardware access module
- Network protocol module
- Logical filesystem module
- Memory management module
- Task scheduler module
- HCI class driver module

Stage 3 (C++):
- Python extension module
- Device model module
- Page cache module
- Swap module
- Memory mapping module
- Syncronization module
- Interpreter module
- Compilation module

Stage 4 (Python):
- Proc module
- Filesystem module
- Protocol family module
- Virtual filesystem module
- Virtual memory module
- Tasks module
- Input subsystems module

Stage 5 (Python):
- System calls module
- Sockets module
- Files and directories module
- Memory Access module
- Processes module
- Video devices module

User level:

Stage 6 (Python):
- Bash shell module
- System package manager module
- Python package manager module
- Linux compatability module
- System updates module
- User accounts module
- Security module
- Extendability module

Stage 7 (Python):
- GUI interface module
