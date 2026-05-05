# ADNU Manuscript Folder Organization

Keep source files and generated files separated.

## Source Folders

- `main.tex`: full manuscript build file.
- `references.bib`: BibTeX database for future citation cleanup.
- `style/`: shared LaTeX formatting used by the full build and all fast builds.
- `misc/`: front matter, back matter, manual references, and revision notes.
- `chapters/chapter_1/` through `chapters/chapter_5/`: chapter source files.
- `appendices/`: Appendix A through Appendix I source files.
- `figures/ch3/`, `figures/ch4/`, and `figures/apx/`: manuscript figures.
- `build/`: fast-build wrappers and the build script only.

## Generated Folders

- `logs/`: LaTeX auxiliary files, logs, tables of contents, lists, and sync files.
- `pdf/`: generated PDFs for the full manuscript and standalone builds.

Generated files should not be edited manually. If the editor shows many red
generated files, clean and rebuild through `build/build_all_pdfs.ps1`.
