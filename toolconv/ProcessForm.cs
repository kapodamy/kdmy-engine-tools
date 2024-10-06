using Settings;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Media;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace ToolConv;

internal partial class ProcessForm : Form
{
    private static readonly IniKeyType[] BNO_ARGUMENTS_GETTER =
    {
        new("whitelist", String.Empty),
        new("blacklist", String.Empty),
        new("no-spaces", false)
    };
    private static readonly IniKeyType[] KDT_ARGUMENTS_GETTER =
    {
        new ("quality", 500),
        new ("dither-algorithm", "auto"),
        new ("scale-algorithm", "lanczos"),
        new ("rgb565"),
        new ("vq"),
        new ("no-twiddled"),
        new ("lzss"),
        new ("uper-slice"),
        new ("uper-slice-max-blocks", 3),
        new ("sub-slice"),
        new ("force-vq-on-small"),
        new ("force-square-vq"),
        new ("downscale-procedure", "CLAMP"),
        new ("pixel-format", "AUTO"),
        new ("scale-factor", 0.0f),
        new ("scale-factor-limit", 16),
        new ("image-magick-exec", ""),
        new ("opacity-slice"),
        new ("max-dimmen", 0)
    };
    private static readonly IniKeyType[] KDM_ARGUMENTS_GETTER =
    {
        new ("hq"),
        new ("dither-algorithm", "auto"),
        new ("scale-algorithm", "lanczos"),
        new ("cue-interval", 3),
        new ("fps", "0"),
        new ("small-resolution"),
        new ("gop", -1),
        new ("no-save-original-resolution"),
        new ("two-pass-bitrate", 5000),
        new ("mpeg-bitrate", -1),
        new ("silence"),
        new ("sample-rate", 0),
        new ("mono"),
        new ("no-progress")
    };
    private static readonly IniKeyType[] SFX_ARGUMENTS_GETTER =
    {
        new ("max-duration", 0),
        new ("sample-rate", -1),
        new ("auto-sample-rate"),
        new ("pcm-u8"),
        new ("force-mono"),
        new ("copy-if-rejected"),
        new ("test-only")
    };

    private static readonly byte[] KDM_INTERPROC_SIGNATURE = Encoding.ASCII.GetBytes("!KDM!@IntrPRC<%>");

    private TaskbarProgressBar taskbarProgress;
    private ProcessedItem last_processed_item;
    private double long_task_current_progress;
    private Stopwatch long_task_stopwatch;
    private ProcessedItem[] processedItems;
    private double[] last_elapsed_seconds;
    private int last_elapsed_seconds_index;

    private string[] bno_arguments;
    private string[] kdt_arguments;
    private string[] kdm_arguments;
    private string[] sfx_arguments;
    private string input_path;
    private string output_path;


    public ProcessForm()
    {
        InitializeComponent();
        last_processed_item = null;
        long_task_stopwatch = new Stopwatch();
        last_elapsed_seconds = new double[15];
        last_elapsed_seconds_index = 0;
    }


    private static string[] BuildArguments(INI settings, string section, IniKeyType[] arguments_getter)
    {
        List<string> list = new List<string>(arguments_getter.Length);

        foreach (IniKeyType argument in arguments_getter)
        {
            string final_value;

            if (argument.optional_value is Boolean optional_boolean)
            {
                bool value = settings.GetBool(section, argument.name, optional_boolean);
                if (value == optional_boolean) continue;
                final_value = null;
            }
            else if (argument.optional_value is Int32 optional_integer)
            {
                int value = settings.GetInt(section, argument.name, optional_integer);
                if (value == optional_integer) continue;
                final_value = value.ToString(CultureInfo.InvariantCulture);
            }
            else if (argument.optional_value is Single optional_float)
            {
                float value = settings.GetFloat(section, argument.name, optional_float);
                if (value == optional_float) continue;
                final_value = value.ToString(CultureInfo.InvariantCulture);
            }
            else if (argument.optional_value is string optional_string)
            {
                string value = settings.GetString(section, argument.name, optional_string);
                if (value == optional_string) continue;
                final_value = value;
            }
            else
            {
                throw new NotImplementedException(
                    "Unknown 'optional_value' type: " + argument.optional_value.GetType().FullName
                );
            }

            list.Add("--" + argument.name);
            if (final_value != null) list.Add(final_value);
        }

        // input and output file placeholders
        list.Add(String.Empty);
        list.Add(String.Empty);

        return list.ToArray();
    }

    private static string PackArguments(string[] arguments)
    {
        int chars = 0;
        for (int i = 0; i < arguments.Length; i++) chars += arguments[i].Length;

        StringBuilder result = new StringBuilder(chars + (arguments.Length * 3));

        foreach (string argument in arguments)
        {
            bool is_escaped = true;
            for (int i = 0; i < argument.Length; i++)
            {
                if (char.IsWhiteSpace(argument[i]) || argument[i] == '"')
                {
                    is_escaped = false;
                    break;
                }
            }

            if (result.Length > 0) result.Append(' ');

            if (is_escaped)
            {
                result.Append(argument);
                continue;
            }

            result.Append('"');

            int index = 0;
            while (index < argument.Length)
            {
                char c = argument[index++];

                if (c != '\\')
                {
                    if (c == '"') result.Append('\\');
                    result.Append(c);
                    continue;
                }

                int escapes = 1;
                while (index < argument.Length && argument[index] == '\\')
                {
                    index++;
                    escapes++;
                }

                if (index == argument.Length)
                    escapes *= 2;
                else if (argument[index] == '"')
                    escapes = (escapes * 2) + 1;

                result.Append('\\', escapes);

                if (argument[index] == '"')
                {
                    result.Append('"');
                    index++;
                }
            }

            result.Append('"');
        }

        return result.ToString();
    }

    private static void AppendToBuffer(StringBuilder buffer, char[] contents, int count, ref int write_index, bool ignore_carrier_return)
    {
        int max_length = buffer.Length;

        if (ignore_carrier_return)
        {
            for (int i = 0; i < count; i++)
            {
                if (write_index >= max_length)
                {
                    count -= i;
                    buffer.Append(contents, i, count);
                    write_index += write_index;
                    break;
                }

                char c = contents[i];
                if (c != '\r') buffer[write_index++] = c;
            }
            return;
        }

        for (int i = 0; i < count; i++)
        {
            char c = contents[i];

            if (c == '\r')
            {
                /*int next_i = i + 1;
                if (next_i < count && contents[next_i] == '\n')
                {
                    continue;
                }*/

                if (write_index < max_length)
                {
                    int last_new_line = 0;
                    for (int j = write_index; j >= 0; j--)
                    {
                        if (buffer[j] == '\n')
                        {
                            last_new_line = j + 1;
                            break;
                        }
                    }
                    write_index = last_new_line;
                }

                continue;
            }
            else if (c == '\n')
            {
                for (; write_index < max_length; write_index++)
                {
                    if (buffer[write_index] == '\n')
                    {
                        break;
                    }
                }

                if (write_index < max_length)
                {
                    write_index++;
                }
                else
                {
                    buffer.Append(c);
                    write_index++;
                    max_length++;
                }

                continue;
            }

            if (write_index < max_length)
            {
                buffer[write_index++] = c;
            }
            else
            {
                buffer.Append(c);
                write_index++;
                max_length++;
            }
        }
    }

    internal void SetItemsToProcess(ListView.ListViewItemCollection items, INI settings, string input_path, string output_path, Form parent)
    {
        processedItems = new ProcessedItem[items.Count];
        for (int i = 0; i < items.Count; i++)
        {
            processedItems[i] = new ProcessedItem() { item = items[i], stdout = new StringBuilder(128) };
        }

        bno_arguments = BuildArguments(settings, "BNO", BNO_ARGUMENTS_GETTER);
        kdt_arguments = BuildArguments(settings, "KDT", KDT_ARGUMENTS_GETTER);
        kdm_arguments = BuildArguments(settings, "KDM", KDM_ARGUMENTS_GETTER);
        sfx_arguments = BuildArguments(settings, "SFX", SFX_ARGUMENTS_GETTER);

        this.input_path = input_path;
        this.output_path = output_path;

        taskbarProgress = new TaskbarProgressBar((parent != null && parent.Visible) ? parent.Handle : this.Handle);
        progressBar.Maximum = taskbarProgress.MaximumValue = processedItems.Length;
        taskbarProgress.State = TaskbarProgressBar.ProgressBarState.Normal;
    }

    private void EnableLongTask(bool enable)
    {
        taskbarProgress.State = enable ? TaskbarProgressBar.ProgressBarState.WarningOrPause : TaskbarProgressBar.ProgressBarState.Normal;
        taskbarProgress.MaximumValue = enable ? progressBarLongTask.Maximum : progressBar.Maximum;
        taskbarProgress.Value = enable ? 0 : progressBar.Value;

        labelLongTaskEstimated.Visible = labelLongTaskHint.Visible = progressBarLongTask.Visible = enable;

        labelLongTaskEstimated.Text = "";

        long_task_stopwatch.Reset();
        for (int i = 0; i < last_elapsed_seconds.Length; i++) last_elapsed_seconds[i] = 0.0;
    }

    private string OpenKDMEnc(Process process, out nint hnd, out nint porj_address)
    {
        // wait a bit before adquire the module base address
        Thread.Sleep(100);
        nint base_address = process.MainModule.BaseAddress;

        hnd = OpenProcess(PROCESS_WM_READ, false, process.Id);
        if (hnd == 0x00)
        {
            porj_address = 0x00;
            return "failed to open the process memory of " + process.MainModule.FileName + "\n\n";
        }

        byte[] buffer = new byte[64 * 1024];
        int attemps = 10;

    L_search:
        int progress = 0;
        nint address = base_address;
        nint end_address = (nint)(base_address + process.PeakWorkingSet64);
        while (address < end_address)
        {
            nint readed = 0;
            if (!ReadProcessMemory(hnd, address, buffer, buffer.Length, ref readed))
            {
                // ¿EOF reached?
                break;
            }

            for (int i = 0; i < readed; i++)
            {
                if (KDM_INTERPROC_SIGNATURE[progress] == buffer[i])
                {
                    progress++;
                    if (progress >= KDM_INTERPROC_SIGNATURE.Length)
                    {
                        // interprocess area found
                        porj_address = address + i + 1;
                        return null;
                    }
                }
                else
                {
                    // find again
                    progress = 0;
                }
            }

            // explore futher
            address += readed;
        }

        if (attemps-- > 0)
        {
            Thread.Sleep(1000);
            goto L_search;
        }

        CloseHandle(hnd);
        hnd = 0x00;

        porj_address = 0x00;
        return "failed to adquire the interprocess area memory of " + process.MainModule.FileName + ", the conversion progress will be not available\n\n";
    }

    private double ReadKDMEnc(nint hnd, nint porj_address)
    {
        byte[] buffer = new byte[sizeof(double)];
        nint readed = 0;

        if (ReadProcessMemory(hnd, porj_address, buffer, buffer.Length, ref readed) && readed == buffer.Length)
        {
            return BitConverter.ToDouble(buffer, 0);
        }

        return 0.0;
    }


    private void listBoxProcessed_SelectedIndexChanged(object sender, EventArgs e)
    {
        ProcessedItem item = (ProcessedItem)listBoxProcessed.SelectedItem;
        textBoxStdOut.Text = item == null ? "" : item.stdout.ToString().Replace("\n", "\r\n");

        if (item != null)
        {
            labelFile.Text = Path.GetFileName((string)item.item.Tag);

            if (item.failed)
                labelStatus.Text = "Failed";
            else if (item.completed)
                labelStatus.Text = item.item.ImageIndex == MainForm.ICON_FILE ? "Copied" : "Completed";
            else
                labelStatus.Text = "Processing";
        }
        else
        {
            labelStatus.Text = "";
            labelFile.Text = "";
        }
    }

    private void buttonCancel_Click(object sender, EventArgs e)
    {
        if (this.AcceptButton == buttonCancel)
        {
            this.Close();
            return;
        }

        DialogResult ret = MessageBox.Show(
            "¿Cancel operation?", this.Text, MessageBoxButtons.YesNo, MessageBoxIcon.Question
        );
        if (ret != DialogResult.Yes) return;

        backgroundWorker.CancelAsync();
        this.Text = "[Canceled] " + this.Text;
        taskbarProgress.State = TaskbarProgressBar.ProgressBarState.Marquee;
        this.progressBar.Style = ProgressBarStyle.Marquee;
    }

    private void timer_Tick(object sender, EventArgs e)
    {
        if (progressBarLongTask.Visible && long_task_current_progress > 0.0)
        {
            if (!long_task_stopwatch.IsRunning) long_task_stopwatch.Start();

            progressBarLongTask.Value = (int)Math.Min(long_task_current_progress * 100.0, 10000.0);
            taskbarProgress.Value = progressBarLongTask.Value;

            double estimated = (long_task_stopwatch.ElapsedTicks / long_task_current_progress) * (100.0 - long_task_current_progress);

            last_elapsed_seconds[last_elapsed_seconds_index++] = estimated;
            if (last_elapsed_seconds_index >= last_elapsed_seconds.Length) last_elapsed_seconds_index = 0;

            estimated = 0.0;
            for (int i = 0; i < last_elapsed_seconds.Length; i++) estimated += last_elapsed_seconds[i];

            estimated = estimated / last_elapsed_seconds.Length / Stopwatch.Frequency;
            if (Double.IsNaN(estimated) || Double.IsInfinity(estimated)) estimated = 0.0;

            TimeSpan time = TimeSpan.FromSeconds(estimated);

            if (time.TotalHours > 0)
                labelLongTaskEstimated.Text = time.ToString("h'h 'mm'm 'ss's'");
            else
                labelLongTaskEstimated.Text = time.ToString("mm'min 'ss's'");
        }

        if (last_processed_item != null && last_processed_item == listBoxProcessed.SelectedItem)
        {
            lock (last_processed_item.stdout)
            {
                textBoxStdOut.Text = last_processed_item.stdout.ToString();
            }
        }
    }

    private void ProcessForm_Shown(object sender, EventArgs e)
    {
        taskbarProgress.Visible = true;
        backgroundWorker.RunWorkerAsync();
        timer.Start();
    }

    private void ProcessForm_FormClosing(object sender, FormClosingEventArgs e)
    {
        this.taskbarProgress.Visible = false;

        if (!backgroundWorker.IsBusy) return;

        switch (e.CloseReason)
        {
            case CloseReason.UserClosing:
            case CloseReason.WindowsShutDown:
            case CloseReason.TaskManagerClosing:
                e.Cancel = true;
                buttonCancel_Click(sender, e);
                break;
        }
    }

    private void backgroundWorker_ProgressChanged(object sender, ProgressChangedEventArgs e)
    {
        if (last_processed_item != null)
        {
            int index = listBoxProcessed.Items.IndexOf(last_processed_item);
            last_processed_item.completed = true;
            listBoxProcessed.Items[index] = listBoxProcessed.Items[index];

            progressBar.Value = /*taskbarProgress.Value = */e.ProgressPercentage - 1;

            if (last_processed_item.completed && !last_processed_item.failed)
            {
                last_processed_item.item.Remove();
            }
        }

        if (e.ProgressPercentage < processedItems.Length)
        {
            last_processed_item = processedItems[e.ProgressPercentage];
            listBoxProcessed.Items.Add(last_processed_item);

            if (listBoxProcessed.SelectedIndex < 0)
            {
                listBoxProcessed.SelectedIndex = 0;
            }
            else if (listBoxProcessed.SelectedIndex == (listBoxProcessed.Items.Count - 2))
            {
                listBoxProcessed.SelectedIndex++;
            }
        }

        labelCompleted.Text = $"{e.ProgressPercentage} / {processedItems.Length}";

        EnableLongTask(last_processed_item.item.ImageIndex == MainForm.ICON_KDM);
    }

    private void backgroundWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
    {
        timer.Stop();

        buttonCancel.Text = (string)this.buttonCancel.Tag;
        this.AcceptButton = this.CancelButton = buttonCancel;

        if (progressBarLongTask.Visible)
        {
            taskbarProgress.State = TaskbarProgressBar.ProgressBarState.Normal;
            taskbarProgress.Value = 0;
            taskbarProgress.MaximumValue = progressBar.Maximum;
            taskbarProgress.Value = 0;
        }

        if (e.Cancelled)
        {
            taskbarProgress.State = TaskbarProgressBar.ProgressBarState.Error;
            this.progressBar.Style = ProgressBarStyle.Continuous;
            TaskbarProgressBar.SetProgressBarState(progressBar, TaskbarProgressBar.ProgressBarState.Error);
        }
        else if (e.Error != null)
        {
            taskbarProgress.State = TaskbarProgressBar.ProgressBarState.Error;
            TaskbarProgressBar.SetProgressBarState(progressBar, TaskbarProgressBar.ProgressBarState.Error);

            SystemSounds.Exclamation.Play();

            if (last_processed_item != null)
            {
                last_processed_item.failed = true;
                last_processed_item.stdout.Append("\r\n\r\n");
                last_processed_item.stdout.Append(e.Error.Message);
                listBoxProcessed.SelectedIndex = Array.IndexOf(processedItems, last_processed_item);
                listBoxProcessed_SelectedIndexChanged(sender, e);
            }
            else
            {
                textBoxStdOut.Text = e.Error.Message;
            }

            MessageBox.Show(e.Error.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
        else
        {
            progressBar.Value++;
            taskbarProgress.Visible = false;
            this.Text = (string)this.Tag;

            SystemSounds.Beep.Play();
        }
    }

    private void backgroundWorker_DoWork(object sender, DoWorkEventArgs e)
    {
        using StreamWriter log_file = new StreamWriter(output_path + Path.DirectorySeparatorChar + "result.log");

        for (int i = 0; i < processedItems.Length; i++)
        {
            if (backgroundWorker.CancellationPending)
            {
                e.Cancel = true;
                return;
            }

            backgroundWorker.ReportProgress(i);

            int type = processedItems[i].item.ImageIndex;
            string path = ((string)processedItems[i].item.Tag);

            string executable;
            string[] arguments;
            bool long_task = false;
            string dest_path = output_path;

            if (input_path.Length > 0)
            {
                if (path.StartsWith(input_path, StringComparison.OrdinalIgnoreCase))
                {
                    string sub_path = Path.GetDirectoryName(path.Substring(input_path.Length));
                    if (String.IsNullOrEmpty(sub_path))
                    {
                        dest_path += Path.DirectorySeparatorChar;
                    }
                    else
                    {
                        dest_path += sub_path;
                        if (sub_path[sub_path.Length - 1] != Path.DirectorySeparatorChar) dest_path += Path.DirectorySeparatorChar;
                    }
                    dest_path += Path.GetFileNameWithoutExtension(path);
                }
                else
                {
                    dest_path += Path.DirectorySeparatorChar + Path.GetFileNameWithoutExtension(path);
                }
            }
            else
            {
                dest_path += Path.DirectorySeparatorChar + Path.GetFileNameWithoutExtension(path);
            }

            string file_extension = Path.GetExtension(path);
            StringBuilder std = processedItems[i].stdout;

            string parent_folder = Path.GetDirectoryName(dest_path);
            Directory.CreateDirectory(parent_folder);

            switch (type)
            {
                case MainForm.ICON_BNO:
                    executable = "bno";
                    arguments = bno_arguments;
                    if (file_extension.Equals(".json", StringComparison.InvariantCultureIgnoreCase))
                        dest_path += ".jbno";
                    else
                        dest_path += ".bno";
                    break;
                case MainForm.ICON_KDT:
                    executable = "kdt_enc";
                    arguments = kdt_arguments;
                    dest_path += ".kdt";
                    break;
                case MainForm.ICON_KDM:
                    executable = "kdm_enc";
                    arguments = kdm_arguments;
                    dest_path += ".kdm";
                    long_task = true;
                    break;
                case MainForm.ICON_SFX:
                    executable = "sfx";
                    arguments = sfx_arguments;
                    dest_path += ".wav";
                    break;
                default:
                    dest_path += file_extension;

                    if (Path.GetFullPath(path) == Path.GetFullPath(dest_path))
                    {
                        continue;
                    }

                    try
                    {
                        File.Copy(path, dest_path, true);
                        lock (std)
                        {
                            std.Append("File '");
                            std.Append(path);
                            std.Append("' copied to '");
                            std.Append(dest_path);
                            std.Append('\'');
                        }
                    }
                    catch (Exception ex)
                    {
                        lock (std)
                        {
                            std.Append("Failed to copy '");
                            std.Append(path);
                            std.Append("' to '");
                            std.Append(dest_path);
                            std.Append("'\r\n ");
                            std.Append(ex.Message);
                        }
                        processedItems[i].failed = true;
                    }

                    processedItems[i].completed = true;
                    continue;
            }

            arguments[arguments.Length - 2] = path;
            arguments[arguments.Length - 1] = dest_path;

            using (Process process = new Process())
            {

                StdCallback callback = delegate (bool stdout_or_stderr, string string_data)
                {
                    lock (std)
                    {
                        std.AppendLine(string_data);
                    }
                };

                process.StartInfo.FileName = executable;
                process.StartInfo.CreateNoWindow = true;
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.StandardOutputEncoding = Encoding.UTF8;
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.StandardErrorEncoding = Encoding.UTF8;
                process.StartInfo.RedirectStandardError = true;
                process.StartInfo.ErrorDialog = true;
                process.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
                process.StartInfo.Arguments = PackArguments(arguments);

                log_file.WriteLine($"{executable} {process.StartInfo.Arguments}\r\n");

                process.OutputDataReceived += (_, e) => callback(true, e.Data);
                process.ErrorDataReceived += (_, e) => callback(false, e.Data);

                process.Start();

                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                nint long_task_hnd = 0x00;
                nint long_task_addr = 0x00;

                if (long_task)
                {
                    try
                    {
                        string open_error = OpenKDMEnc(process, out long_task_hnd, out long_task_addr);
                        if (open_error != null)
                        {
                            callback(false, open_error);
                        }
                    }
                    catch (Exception ex)
                    {
                        callback(true, ex.Message + "\n" + ex.StackTrace);
                    }
                }

                while (!process.HasExited)
                {
                    if (long_task_hnd != 0x00)
                    {
                        try
                        {
                            long_task_current_progress = ReadKDMEnc(long_task_hnd, long_task_addr);
                            Thread.Sleep(200);
                        }
                        catch (Exception ex)
                        {
                            Console.Error.WriteLine(ex.Message + "\n" + ex.StackTrace);
                        }
                    }

                    if (backgroundWorker.CancellationPending)
                    {
                        e.Cancel = true;
                        process.Kill();
                        return;
                    }
                }

                if (long_task_hnd != 0x00) CloseHandle(long_task_hnd);

                processedItems[i].failed = process.ExitCode != 0;
                processedItems[i].completed = true;

                //callback(true, process.StandardOutput.ReadToEnd());
                //callback(false, process.StandardError.ReadToEnd());

                process.WaitForExit();
                process.Close();

                log_file.WriteLine(std.ToString());
                log_file.WriteLine("--------------------------------------------------\r\n\r\n");
            }
        }

        backgroundWorker.ReportProgress(processedItems.Length);
    }

    const int PROCESS_WM_READ = 0x0010;
    [DllImport("kernel32")] public static extern nint OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);
    [DllImport("kernel32")] public static extern bool ReadProcessMemory(nint hProcess, nint lpBaseAddress, byte[] lpBuffer, nint dwSize, ref nint lpNumberOfBytesRead);
    [DllImport("kernel32")] public static extern nint CloseHandle(nint hObject);


    private delegate void StdCallback(bool stdout_or_stderr, string string_data);


    private class ProcessedItem
    {
        public ListViewItem item;
        public StringBuilder stdout;
        public volatile bool completed;
        public volatile bool failed;

        public string Text
        {
            get
            {
                if (failed)
                    return "✘ " + item.Text;
                else if (completed)
                    return "✔ " + item.Text;
                else
                    return "  " + item.Text;
            }
        }

    }

    private struct IniKeyType
    {
        public readonly string name;
        public readonly object optional_value;

        public IniKeyType(string value_name, object optional_value)
        {
            this.name = value_name;
            this.optional_value = optional_value;
        }

        public IniKeyType(string boolean_name)
        {
            this.name = boolean_name;
            this.optional_value = false;
        }

    }

}
