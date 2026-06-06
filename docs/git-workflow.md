# Team Git Workflow

## Why We Use This Workflow
This workflow helps us work in parallel without breaking each other's code.

- Documentation changes are fast and simple.
- Code changes are reviewed and safer.
- The main branch stays clean and working.

## Branches We Use
- main: stable branch, only clean and working changes.
- dev: integration branch for code work.
- docs/...: branches used only for documentation updates.
- feat/...: feature branches for new code, created from dev.
- fix/...: bug fix branches for code, created from dev.

## Quick Rules
1. If you edit documentation, create a docs/... branch.
2. If you edit code, start from dev and create feat/... or fix/... branch.
3. Pull Requests are required for code work that goes to dev.
4. Pull Requests are not required for docs/... branches.
5. main only gets clean, tested, working changes.

## Documentation Workflow (No PR Required)
Use this when you only change files inside docs.

1. Update local main:
```bash
git checkout main
git pull
```

2. Create a documentation branch:
```bash
git checkout -b docs/update-http-notes
```

3. Edit documentation files.

4. Commit and push:
```bash
git add docs
git commit -m "docs: update HTTP notes"
git push -u origin docs/update-http-notes
```

5. Merge docs branch using team rule (PR not required).

### Docs branch name examples
- docs/add-cgi-notes
- docs/fix-typos
- docs/rewrite-architecture-section

## Code Workflow (PR Required)
Use this for any code change.

1. Start from dev:
```bash
git checkout dev
git pull
```

2. Create your work branch:
```bash
git checkout -b feat/request-parser
```
or
```bash
git checkout -b fix/socket-timeout
```

3. Work, commit, and push:
```bash
git add .
git commit -m "feat: add request parser"
git push -u origin feat/request-parser
```

4. Open a Pull Request to dev.

5. Merge only after checks pass.

## How Changes Reach main
When code in dev is clean and working, move it to main.

Typical release flow:
1. Merge feat/... and fix/... branches into dev with PR.
2. Test and validate dev.
3. Merge dev into main.

## Full Example
### Example A: Documentation only
- Create docs/add-poll-examples from main.
- Edit only docs files.
- Commit and push.
- Merge without PR (team rule).

### Example B: Code change
- Create feat/http-response-builder from dev.
- Implement code.
- Push branch.
- Open PR to dev.
- Merge after checks.

### Example C: Release
- Team confirms dev is stable.
- Merge dev into main.

## Tips to Avoid Conflicts
- Do not mix docs and code in the same branch.
- Keep branches small and short-lived.
- Pull before starting new work.
- Use clear commit messages.
- If your branch is old, update it with latest dev before PR.

## Commit Message Examples
- docs: clarify epoll section
- docs: add CGI beginner notes
- feat: add response header builder
- fix: close socket on disconnect
