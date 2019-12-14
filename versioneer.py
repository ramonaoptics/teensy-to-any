import subprocess

revision = subprocess.check_output(["git", "describe", "--dirty"]).strip()
revision = revision.decode()
print('-DGIT_DESCRIBE=\'"%s"\'' % revision)

