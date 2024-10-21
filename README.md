# PeakSat COMMS Software

### Developer Setup
To set-up and run pre-commit you need to:
```sh
pip install pre-commit
pre-commit install # This should be executed inside this repository

# Execute the following in order to test the installation
pre-commit run 
```
**Note:** For Linux users it is recommended for pre-commit to be installed inside a virtual environment.

Before each commit, `clang-format` will be ran locally in order to fix codestyle and formatting.