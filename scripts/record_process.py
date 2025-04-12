from cyber_record.record import Record
import time
            
def read_write_message():
    read_file_name = "/home/huang/Cyber_Records/20250307160350.record.00000"
    r_record = Record(read_file_name)

    write_file_name = "/home/huang/Cyber_Records/20250307160350_out.record.00000"
    with Record(write_file_name, mode='w') as w_record:
        for topic, message, t in r_record.read_messages_fallback():
            print("{}, {}, {}".format(topic, type(message), t))
            w_record.write(topic, message, t)

if __name__ == "__main__":
    read_write_message()
