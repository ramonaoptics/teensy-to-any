import subprocess

revision = subprocess.check_output(["git", "describe", "--tags", "--dirty"]).strip()
revision = revision.decode()
revision = revision.replace('-dirty', '+dirty', 1)
revision = revision.replace('-', '.post', 1)
revision = revision.replace('-g', '+g', 1)

print('-DGIT_DESCRIBE=\'"%s"\'' % revision)
