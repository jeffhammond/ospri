OSPRI
=====

One-Sided Primitives

# Summary

OSPRI is a new one-sided runtime communication system for leadership-class supercomputers.  It aims to provide more flexible semantics compared to existing one-sided communication systems (e.g. [ARMCI](http://www.emsl.pnl.gov/docs/parsoft/armci/)) and target modern architectural trends by providing a thread-safe implementation.

The proper citation of OSPRI is:

OSPRI: J. R. Hammond, J. Dinan, P. Balaji, I. Kabadshow, S. Potluri, and V. Tipparaju, ''The 6th Conference on Partitioned Global Address Space Programming Models'' (PGAS). Santa Barbara, CA, October 2012. "OSPRI: An Optimized One-Sided Communication Runtime for Leadership-Class Machines."

The corresponding Bibtex is:

```
@inproceedings{Hammond:PGAS:2012:OSPRI,
   author = {Jeff R. Hammond and James Dinan and Pavan Balaji and Ivo Kabadshow and Sreeram Potluri and Vinod Tipparaju},
   title = {{OSPRI}: An Optimized One-Sided Communication Runtime for Leadership-Class Machines},
   booktitle = {The 6th Conference on Partitioned Global Address Space Programming Models ({PGAS})},
   month = oct,
   year = {2012},
   location = {Santa Barbara, CA, USA},
   url = {http://www.mcs.anl.gov/publications/paper_detail.php?id=1545},
}
```

Please see the [preprint](http://www.mcs.anl.gov/publication/ospri-optimized-one-sided-communication-runtime-leadership-class-machines) for details.

# Goals of the project

* maximum communication performance
* scalability to millions of processes
* thread-safety
* support for hybrid systems (e.g. GPUs)
* modular design ala MPI
* software quality (readable, documented source code and robust build system)
* productive interaction with vendors

# Developers

Jeff Hammond, Sayan Ghosh, Sreeram Potluri, and Pavan Balaji have all contributed to this project.
