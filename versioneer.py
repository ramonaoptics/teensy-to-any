import subprocess

revision = subprocess.check_output(["git", "describe", "--dirty"]).strip()
print('-DGIT_DESCRIBE=\'"%s"\'' % revision)