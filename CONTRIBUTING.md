# How to contribute

Follow these guidelines to help the process of accepting merges to go smoothly
for everyone:

1. Clone/fork the repository.

2. Create a new branch (`git checkout -b your_branch`).
   Name the branch `bugfix/xxx`, if the contribution is a bug fix. Else, name
   the branch `feature/xxx`. Consistent naming of branches makes the
   output of `git branch` easier to understand with a single glance.

3. Do modifications on that branch. Except for special cases, the
   contribution should include proper unit tests and documentation. Follow the
   C++ and style guide below.

4. Make sure the modifications did not break anything by building and
   running the tests.

5. Check the quality of tests by collecting coverage. Make sure the tests cover
   _100%_ of the new code.

6. Commit the changes. Follow the Git commit guide below.

7. Push the changes to the remote branch (`git push origin your_branch`).

8. Open a merge request against project's `main` branch.

    1. Follow the link on your screen (after `git push`) to create a merge
       request.
    1. Check "Delete source branch when merge request is accepted". Optionally,
       select a user to review your merge request in the "Assignee" menu.
    1. Proceed with "Submit merge request" button.

9. Collaborate, implement suggestions, amend the branch.

## C++ guide

- Do not use `using` directives in headers in global namespace.
- Use `auto` if initialization already mentions type (like in
  `auto v = std::make_shared<Type>()`).
- Prefer `enum class` to C-style `enum` .
- Use `std::optional<T>` to return a value of a function that may fail.
- Never use exceptions.

## Style guide

Support for `clang-format` will be added in the future.

Style conventions:
- Name case - see the table below.
- Project's `#include`s go first, then a blank line and system headers.
  Sort `#include`s within each block in alphabetical order.
- Avoid prefixes (like `m_`) and suffixes (like `_num` or `_str`) where
  possible.
- If in doubt, use your own judgment and stick to the style of the surrounding
  code.

| Name                  | Convention                                    |
|-----------------------|-----------------------------------------------|
| Git branch            | `feature/branch-name` or `bugfix/branch-name` |
| File/directory        | `snake_case`                                  |
| Class                 | `snake_case`                                  |
| Variable              | `snake_case`                                  |
| Function              | `snake_case`                                  |
| Macro/static constant | `CAPITAL_SNAKE_CASE`                          |
| Template parameter    | `CamelCase`                                   |

## Git commit guide

These guidelines are an excerpt from the
[Pro Git Book](https://git-scm.com/book/en/v2/Distributed-Git-Contributing-to-a-Project).

1. Submissions should not contain any whitespace errors. Git provides an easy
   way to check for this — before you commit, run `git diff --check`, which
   identifies possible whitespace errors and lists them for you.

2. Try to make each commit a logically separate changeset. If you can, try to
   make your changes digestible — don’t code for a whole weekend on five
   different issues and then submit them all as one massive commit on Monday.
   This approach also makes it easier to pull out or revert one of the
   changesets if you need to later.

3. Write a good commit message. As a general rule, your messages should start
   with a single line that’s no more than about 50 characters and that describes
   the changeset concisely, followed by a blank line, followed by a more
   detailed explanation. Write your commit message in the imperative: "Fix bug"
   and not "Fixed bug" or "Fixes bug."

Model commit message
```
Short (50 chars or less) summary

More detailed explanatory text, if necessary. Wrap it to about 78
characters or so. In some contexts, the first line is treated as the
subject of an email and the rest of the text as the body. The blank
line separating the summary from the body is critical (unless you omit
the body entirely); tools like rebase can get confused if you run the
two together.

Write your commit message in the imperative: "Fix bug" and not "Fixed bug"
or "Fixes bug." This convention matches up with commit messages generated
by commands like git merge and git revert.

Further paragraphs come after blank lines.

- Bullet points are okay, too

- Typically a hyphen or asterisk is used for the bullet, preceded by a
  single space, with blank lines in between, but conventions vary here

- Use a hanging indent
```

Another example of acceptable commit message
```
Refactor the interface

- Rename elem to contains
- Rename subset to is_subset, and make is_subset applicable in infix notation
- Add the at_key method
- operator[] is now bound to at_key instead of find
```