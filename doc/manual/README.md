# Far Horizons Game Manual

The original [plain text manual](../rules) for Far Horizons (the 7th edition)
were converted to LaTeX by Casey Link in March 2011.

The original LaTeX environment he used has been lost, so getting that pixel
perfect result is no longer possible.

But a perfectly servicable version is available thanks to pandoc.

To rengerate the docs you'll need podman or docker installed, then

```bash
cd latex
make all

# for podman user
make all DOCKER=podman
```
