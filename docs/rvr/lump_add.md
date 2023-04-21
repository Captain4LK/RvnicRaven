# RvR_lump_add

Adds a lump with the specified name to the ressource pool. If the name already exists, it will be overwriten. The specified path can be either the file itself or a .pak file containing the lump.

## Definition

```c
void RvR_lump_add(const char *name, const char *path);
```

## Related

[RvR_pak_add](pak_add)

[RvR_lump_get](lump_get)
