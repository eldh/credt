# rupp


[![CircleCI](https://circleci.com/gh/eldh/rupp/tree/master.svg?style=svg)](https://circleci.com/gh/eldh/rupp/tree/master)


**Contains the following libraries and executables:**

```
rupp@0.0.0
│
├─test/
│   name:    TestRupp.exe
│   main:    TestRupp
│   require: rupp.lib
│
├─library/
│   library name: rupp.lib
│   namespace:    Rupp
│   require:
│
└─executable/
    name:    RuppApp.exe
    main:    RuppApp
    require: rupp.lib
```

## Developing:

```
npm install -g esy
git clone <this-repo>
esy install
esy build
```

## Running Binary:

After building the project, you can run the main binary that is produced.

```
esy x RuppApp.exe 
```

## Running Tests:

```
# Runs the "test" command in `package.json`.
esy test
```
