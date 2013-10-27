Ambience
========

**Warning!!! Do not use this software for production! It may destroy your data! Use this software only for testing!**

![Screenshot](https://raw.github.com/AlekseyDurachenko/Ambience/master/screenshot.png)

Build
=====

download the additional library:
--------------------------------

    mkdir libs
    cd libs
    git clone https://github.com/AlekseyDurachenko/QtSampleEngine qse
    cd ..

compile the project
-------------------

    qmake
    make -j13
    start the program
    cd bin
    ./ambience
    
Feature
-------

- save selection
- delete
- trim
- normalize

Supported formats
-----------------

- see the libsndflie
