import subprocess

revision = subprocess.check_output(["git", "describe", "HEAD"]).strip()
print('-DGIT_DESCRIBE=\'"%s"\'' % revision)