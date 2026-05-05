# Final Formatting Cleanup Checklist

Use this checklist before final export.

## Problems Found in the Current PDF

- Chapter-title spacing was too loose because the report-class chapter display spacing was still dominant.
- Hyperref link borders appeared around cross-references, making text such as table numbers look boxed.
- Standalone chapter/section builds did not import labels from the full manuscript, so some cross-references appeared as `??`.
- The first paragraph after headings was not always indented.
- Captions used default LaTeX colon-style separation instead of the required period after the figure/table number.
- Manual references were indented like normal paragraphs instead of using hanging indentation.
- Appendix C, Appendix D, and Appendix E longtables had converted `\endhead` blocks after the body rows, causing repeated-header and clipping problems.
- Appendix C, Appendix D, and Appendix E contained blank converted sections that produced weird TOC entries.
- Appendix E and Appendix G had section titles with repeated manual appendix numbers.
- Chapter 5 contained a spelling error in the chapter title and needed stronger recommendations.
- Some body text used firmware filenames instead of thesis-friendly technical wording.
- Some measurements used spaces between numbers and units.

## Pre-Export Checks

- Build `pdf/main.pdf` with no undefined references, citations, or missing figures.
- Open `pdf/main.pdf` and check chapter openings, especially Chapters 3, 4, 5, and Appendices C--E.
- Check that table captions appear above tables and figure captions appear below figures.
- Check that the List of Tables and List of Figures use period caption style.
- Check that Appendix G is Letters and Proposals, Appendix H is Gantt Chart, and Appendix I is Cost Breakdown.
- Check that no Appendix J appears.
- Check that no visible `??` appears in `pdf/main.pdf`.
- Check that root source files remain limited to `main.tex`, `references.bib`, and project metadata; generated PDFs should be in `pdf/`.
- Check that generated LaTeX auxiliary and log files are in `logs/`, not mixed with chapter or appendix source files.
- Check that fast-build wrappers remain under `build/misc/`, `build/chapters/`, and `build/appendices/`.
- Check that Chapter 1 through Chapter 5 sources remain in their own `chapters/chapter_N/` folders.
- Check Appendix F against the TinyML reports if the training generation changes.
- Check large appendix tables for readability after the longtable header repair.
- Confirm socket-heating fitted constants against the original characterization worksheet if it becomes available.
