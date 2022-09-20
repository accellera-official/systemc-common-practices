How to Contribute
=================

 This repository is owned by the [Accellera Systems Initiative][1] and
 is maintained by the SystemC Common Practices Working Group
 according to the [Accellera Policies and Procedures][2].

 **Contributions to this reference implementation can only be
   accepted from Accellera members.**

### Join the Accellera SystemC Common Practices Working Group

 If you would like to contribute to the development of one of the SystemC 
 reference implementations, have your company, organization, or university
 join Accellera and its working groups.
 Find out more information at http://www.accellera.org/about/join.
 If your company, organization or university is already an Accellera member,
 you can request to [join the SystemC Language Working Group here][3].

### Join the SystemC community

 If you are not an Accellera member, please join the **[SystemC Forum][4]** 
 to provide feedback, report bugs and join the general
 discussion around the evolution of SystemC and its ecosystem.

---------------------------------------------------------------------
Issue reporting
---------------------------------------------------------------------

You can post issues, bugs and suggestions of general interest to the
[SystemC Forum][4] or [Issue tracker][5].  When reporting bugs, please specify
the following information (if applicable):

  1. SystemC library version
  2. platform, compiler, flags
  3. description of the problem
  4. steps to reproduce the problem
  5. compile/runtime warnings and errors
  6. code sample, not more than 100 lines to demonstrate the problem

> **Note**  
>  All bugs will only be tested against the latest publicly available
>  version of the product.

> **Note**  
>  All C++ compilers that SystemC supports have bugs of different
>  degree of severity. We cannot fix those bugs.
>  Please report them to the compiler vendor.

---------------------------------------------------------------------
Patch submission
---------------------------------------------------------------------

The following **sign-off procedure** is established to ensure that
patches submitted for inclusion into this Accellera reference
implementation are properly licensed under the
[Apache License Version 2.0](LICENSE).

The sign-off is a simple line at the end of the explanation for the
patch (or commit message), which certifies that you wrote it yourself
or otherwise have the right to pass it on as an open-source patch:

### Accellera Developer's Certificate of Origin

By making a signed-off contribution to this Accellera project,
I certify that:

 1. The contribution was created in whole or in part by me and I have
    the right to submit it under the Apache License Version 2.0
    (see LICENSE).

 2. The contribution was provided directly to me by some other person
    who certified (1) above, and I am forwarding it without
    modification.

 3. I understand and agree that this Accellera project and the
    contribution are public and that a record of the contribution
    (including all personal information I submit with it, including
    my sign-off) is maintained indefinitely and may be redistributed
    in accordance with this project or the Apache License Version 2.0.

If you can certify the above *Accellera Developer's Certificate of Origin*,
please use `git commit --signoff` to add a line of the form:
```
  Signed-off-by: Ima Contributor <ima@contributor.example.org>
```
using your real name (no pseudonyms or anonymous contributions).

> **Note**  
> For Accellera members, contributions are already bound by the
> [Accellera policies and procedures][2] and the sign-off is optional,
> but recommended.  For **non-Accellera** members, the sign-off is
> **mandatory** for consideration by the Accellera WGs.

When submitting a pull-request against the public repository, the
contribution may be considered by the Accellera WGs for inclusion. It 
stays under the sole governance
of the corresponding WGs to decide whether a contribution will be included
in the reference implementation (or future Accellera standards).

---------------------------------------------------------------------
Repository organization
---------------------------------------------------------------------

More information on the repository organization can be found in the 
[Development process description](./docs/DEVELOPMENT.md).


[1]: https://www.accellera.org
[2]: https://accellera.org/about/policies-and-procedures
[3]: https://workspace.accellera.org/apps/org/workgroup/lwg/
[4]: https://forums.accellera.org/forum/9-systemc/
[5]: https://github.com/accellera-official/systemc-common-practices/issues