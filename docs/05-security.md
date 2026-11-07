# 5. Secrets and publication safety

## Never commit

Private SSH and host keys, ChatGPT tokens and cookies, the contents of `web-session.json`, raw network captures, URLs with access parameters, certificates with private keys, device dumps and archives, `/data` backups, root toolkit logs, and Wi-Fi or VPN credentials. Do not publish real IP addresses or device or Dot IDs without an informed decision. `.gitignore` helps, but does not prevent forced inclusion or secrets being added to an already tracked file.

## Before every commit and push

```sh
# macOS with Homebrew installed:
brew install gitleaks
./scripts/setup-hooks.sh
```

The local `pre-commit` hook scans staged changes, and `pre-push` scans repository history. If Gitleaks is missing or finds a possible secret, the operation stops. The hooks are stored in the repository, but **you must run setup in every new clone**. Git permits hooks to be disabled or bypassed: do not use `--no-verify`. Also review attachments, binaries, and context manually.

GitHub Actions additionally scans submitted changes after pushes and pull requests. This detects problems after submission; it does not protect data before publication. The original public repository also has built-in Secret scanning and Push protection enabled; check GitHub Settings -> Code security and analysis for your own fork. Even together, these mechanisms cannot guarantee detection of every secret.

## If a secret reaches public Git

Treat it as compromised: immediately revoke or replace it with the provider, terminate sessions, and then work on cleaning history and copies. Deleting the file in a later commit does not remove it from history. Do not send the secret itself when asking for help.
