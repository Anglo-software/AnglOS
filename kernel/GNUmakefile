override KERNEL := angl-os.elf

define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

$(eval $(call DEFAULT_VAR,CC,x86_64-elf-gcc))
$(eval $(call DEFAULT_VAR,CXX,x86_64-elf-g++))
$(eval $(call DEFAULT_VAR,LD,x86_64-elf-ld))

CFLAGS ?= -g -O3 -pipe -Wall -Wextra
CXXFLAGS ?= -g -O3 -pipe -Wall -Wextra
CPPFLAGS ?=
NASMFLAGS ?= -F dwarf -g
LDFLAGS ?=

override CFLAGS +=       \
    -std=c11             \
    -ffreestanding       \
    -fno-stack-protector \
    -fno-stack-check     \
    -fno-lto             \
    -fno-pie             \
    -fno-pic             \
    -m64                 \
    -march=x86-64        \
    -mabi=sysv           \
    -mno-80387           \
    -mno-mmx             \
    -mno-red-zone        \
    -mcmodel=kernel      \
    -MMD                 \
    -masm=intel          \
    -I.

override CXXFLAGS +=     \
    -std=c++11           \
    -ffreestanding       \
    -fno-stack-protector \
    -fno-stack-check     \
    -fno-lto             \
    -fno-pie             \
    -fno-pic             \
    -m64                 \
    -march=x86-64        \
    -mabi=sysv           \
    -mno-80387           \
    -mno-mmx             \
    -mno-red-zone        \
    -mcmodel=kernel      \
    -MMD                 \
    -I.

override LDFLAGS +=         \
    -nostdlib               \
    -static                 \
    -m elf_x86_64           \
    -z max-page-size=0x1000 \
    -T linker.ld

ifeq ($(shell $(LD) --help 2>&1 | grep 'no-pie' >/dev/null 2>&1; echo $$?),0)
    override LDFLAGS += -no-pie
endif

override NASMFLAGS += \
    -f elf64

override CFILES := $(shell find . -type f -name '*.c' | grep -v -E "(userprogs/)")
override CXXFILES := $(shell find . -type f -name '*.cpp' | grep -v -E "(userprogs/)")
override NASMFILES := $(shell find . -type f -name '*.asm' | grep -v -E "(userprogs/)")
override OBJ := $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) $(NASMFILES:.asm=.o)
override HEADER_DEPS := $(CFILES:.c=.d) $(CXXFILES:.cpp=.d)

override PROGS := $(shell find userprogs/ -mindepth 1 -maxdepth 1 -type d)

.PHONY: all
all: $(KERNEL)
	@for i in $(PROGS); do echo "Building" $$i && make -C $$i; done
    
$(KERNEL): $(OBJ)
	$(LD) $(OBJ) $(LDFLAGS) -o $@

-include $(HEADER_DEPS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.asm
	nasm $(NASMFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS)
	@for i in $(PROGS); do echo "Cleaning" $$i && make clean -C $$i; done