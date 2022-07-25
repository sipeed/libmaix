
import time, os, glob, subprocess

print("[adb push & check file]", time.asctime())

def shell(cmd):
    res = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    result = res.stdout.read()
    res.wait()
    res.stdout.close()
    return result.decode("iso8859-1")

def adb_push(dir='./dist/**/*'):
  files = [f for f in glob.glob(dir)]
  for name in files:
    if not os.path.isdir(name):
      old_file = shell('adb shell "sha256sum %s%s"' % ("/root/", name)).replace('./', '').replace('\n', '').replace('\r', '').split('  ')
      new_file = shell('sha256sum %s%s' % ("./", name)).replace('./', '').replace('\n', '').replace('\r', '').split('  ')
      try:
        print('check [%s & %s] file %s' % (new_file[0][:6], old_file[0][:6], old_file[1]))
        if old_file[0] != new_file[0]:
          # print('update %s to %s' % (new_file[1], old_file[1]))
          os.system('adb push %s %s' % (new_file[1], old_file[1]))
      except Exception as e:
        print(shell("adb push dist /root/"))
        print(e)

if "No such file or directory" in shell("adb shell ls /root/res/*"):
  print("first push res")
  print(shell("adb push res /root/"))

# adb_push('./res/**/*') # slow
# adb_push('./res/*') # slow

if "No such file or directory" in shell("adb shell ls /root/maix_dist/*"):
  print("first push dist")
  print(shell("adb push dist /root/"))

adb_push('./dist/**/*')
adb_push('./dist/*')

# print(shell("adb shell 'rm -rf /root/maix_dist && mv /root/dist /root/maix_dist && sync && reboot'"))

