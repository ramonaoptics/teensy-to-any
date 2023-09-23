import subprocess

revision = subprocess.check_output(["git", "describe", "--tags", "--dirty"]).strip()
revision = revision.decode()
revision = revision.replace('-', '.dev', 1)
revision = revision.replace('-', '+', 1)
print('-DGIT_DESCRIBE=\'"%s"\'' % revision)
