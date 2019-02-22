-- lua script
module (..., package.seeall)

pb.import("net.proto")

function main()
  local msg = pb.new("net.tb_Person")
  msg.number = "13615632545"
  msg.email = "13615632545@163.com"
  msg.age = 28
  msg.ptype = 2
  msg.desc:add("first")
  msg.desc:add("second")
  msg.desc:add("three")
  
  local debugstr = pb.tostring(msg)
  local binstr = pb.serializeToString(msg)

  print("msgdebug:\n" .. debugstr)
  print("msgbin:\n" .. binstr)

  local msg2 = pb.new("net.tb_Person")
  pb.parseFromString(msg2, binstr)

  print("msg2debug:\n" .. pb.tostring(msg2))
  print("msg2bin:\n" .. pb.serializeToString(msg2))
end

