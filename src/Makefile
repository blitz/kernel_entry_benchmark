NASM ?= nasm

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

ASFLAGS  += -g -f elf64 -F dwarf

CPPFLAGS  = -nostdinc -I.

CXXFLAGS += -g -O2 -march=x86-64 -mtune=broadwell -std=c++14 -Wall -nostdinc++
CXXFLAGS += -mno-red-zone

# Newer compilers can do -mgeneral-regs-only, but for old ones we have to do:
CXXFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2

CXXFLAGS += -ffreestanding -fno-asynchronous-unwind-tables -fno-threadsafe-statics -fno-rtti -fno-exceptions
CXXFLAGS += -fno-PIC -fno-stack-protector

LDFLAGS = -static -no-pie -g -nostdlib -Wl,--build-id=none -Wl,-n -Wl,-T

OBJS := entry.o io.o main.o start.o

kernel_entry_benchmark.elf32: kernel_entry_benchmark.elf64
	$(OBJCOPY) -O elf32-i386 $< $@
	$(STRIP) $@

kernel_entry_benchmark.elf64: standalone.lds $(OBJS)
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) kernel_entry_benchmark.elf64

%.o : %.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(CXX) -c $(DEPFLAGS) $(CXXFLAGS) $(CFLAGS) $(CPPFLAGS) $< -o $@

%.o : %.asm
	$(NASM) $(ASFLAGS) $< -o $@

$(DEPDIR): ; @mkdir -p $@

DEPFILES := $(SRCS:%.c=$(DEPDIR)/%.d)
$(DEPFILES):
include $(wildcard $(DEPFILES))
