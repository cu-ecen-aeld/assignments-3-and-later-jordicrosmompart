# Analysis of a kernel oops caused by "dev/faulty"

The output of the kernel oops is below, and the analysis will be based on it:

## OOPS Output
```
#  echo “hello_world” > /dev/faulty
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
Mem abort info:
  ESR = 0x96000045
  EC = 0x25: DABT (current EL), IL = 32 bits
  SET = 0, FnV = 0
  EA = 0, S1PTW = 0
  FSC = 0x05: level 1 translation fault
Data abort info:
  ISV = 0, ISS = 0x00000045
  CM = 0, WnR = 1
user pgtable: 4k pages, 39-bit VAs, pgdp=0000000042115000
[0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
Internal error: Oops: 96000045 [#1] SMP
Modules linked in: hello(O) faulty(O) scull(O)
CPU: 0 PID: 156 Comm: sh Tainted: G           O      5.15.18 #1
Hardware name: linux,dummy-virt (DT)
pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
pc : faulty_write+0x14/0x20 [faulty]
lr : vfs_write+0xa8/0x2b0
sp : ffffffc008d13d80
x29: ffffffc008d13d80 x28: ffffff80020d8cc0 x27: 0000000000000000
x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
x23: 0000000040001000 x22: 0000000000000012 x21: 000000556a332a70
x20: 000000556a332a70 x19: ffffff80020a8700 x18: 0000000000000000
x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
x11: 0000000000000000 x10: 0000000000000000 x9 : 0000000000000000
x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
x5 : 0000000000000001 x4 : ffffffc0006f7000 x3 : ffffffc008d13df0
x2 : 0000000000000012 x1 : 0000000000000000 x0 : 0000000000000000
Call trace:
 faulty_write+0x14/0x20 [faulty]
 ksys_write+0x68/0x100
 __arm64_sys_write+0x20/0x30
 invoke_syscall+0x54/0x130
 el0_svc_common.constprop.0+0x44/0xf0
 do_el0_svc+0x40/0xa0
 el0_svc+0x20/0x60
 el0t_64_sync_handler+0xe8/0xf0
 el0t_64_sync+0x1a0/0x1a4
Code: d2800001 d2800000 d503233f d50323bf (b900003f) 
---[ end trace 8cc1d6ed0b3105e2 ]---
```

## Output explanation
The `Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000` clearly explains the reason why the oops has happened: a dereference of a pointer to NULL (0x00) has been attempted. Since this is not a valid address to be accessed, the kernel has printed this output.

The follownig section, `Mem abort info:` and `Data abort info:`, contains register values that have information about the failure, which are already parsed and included in the formatted output.

The next section includes information parsed from the registers previously printed and includes user page table information, error reason, LKM linked, CPU core and PID of the fault. What is most interesting is the content of the CPU registers:
- `pc : faulty_write+0x14/0x20 [faulty]` expresses the PC (program counter) register at the moment of the oops. The function symbol is also interpreted as well as its size. Within the 32 (0x20) bytes of the instruction,  the crash has happened at the offset 20 (0x14).
- `lr : vfs_write+0xa8/0x2b0` expresses the LR (link register) register at the moment of the oops. It expresses what funcion and at what offset the call to "faulty_write" has happened.
- `sp, x29 - x0` are the Stack Pointer and GP registers.

Right after the CPU registers, there is the call trace.

## Debug approach
In order to efficiently use the information provided in the dump, I would follow these steps:
1. Check the PC register to understand where the failure has occured.
2. Having the function and offset, I would use `objdump `, providing the "faulty_write" function:
    ```
    0000000000000000 <faulty_write>:
      0:   d503245f        bti     c
      4:   d2800001        mov     x1, #0x0                        // #0
      8:   d2800000        mov     x0, #0x0                        // #0
      c:   d503233f        paciasp
    10:   d50323bf        autiasp
    14:   b900003f        str     wzr, [x1]
    18:   d65f03c0        ret
    1c:   d503201f        nop
    ```
    Check how the offset 20 (0x14) is where the null pointer is dereferenced. Checking the C code, it is also spotted:
    ```
    ssize_t faulty_write (struct file *filp, const char __user *buf, size_t count,
      loff_t *pos)
    {
      /* make a simple fault by dereferencing a NULL pointer */
      *(int *)0 = 0;
      return 0;
    }
    ```
3. In this module's case, that is enough to trace the null dereference and find the source of the mistake. Nevertheless, other dump data might be useful to debug more difficult failures.