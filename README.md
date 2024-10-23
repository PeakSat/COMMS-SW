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

**Note:** For Windows users, if the above installation does not work, you can try to run the commands by pre-fixing `python -m `. Also some installations specify the `pre-commit` package as `pre_commit`. Run `python -m pip list` to verify it.

Before each commit, `clang-format` will be ran locally in order to fix codestyle and formatting.