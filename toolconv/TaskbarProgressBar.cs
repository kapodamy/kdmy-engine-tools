using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;

/// <summary>
/// Provides manipulation of the progress bar in the taskbar (Only in Windows vista or later).
/// </summary>
public class TaskbarProgressBar
{
    /// <summary>
    /// Creates a new instance using the window handle of the main window of the currently active process.
    /// </summary>
    /// <exception cref="System.NotImplementedException">If the operating system is not Windows Vista or later</exception>
    public TaskbarProgressBar() : this(System.Diagnostics.Process.GetCurrentProcess().MainWindowHandle) { }

    /// <summary>
    /// Creates a new instance using a window handle.
    /// </summary>
    /// <param name="owner">An implementation of the System.Windows.Forms.IWin32Window object that will own this object.</param>
    /// <exception cref="System.NotImplementedException">If the operating system is not Windows Vista or later</exception>
    public TaskbarProgressBar(IWin32Window owner) : this(owner.Handle) { }

    /// <summary>
    /// Creates a new instance using a window handle.
    /// </summary>
    /// <param name="Handle">The Windows.Form handle that will own this object</param>
    /// <exception cref="System.NotImplementedException">If the operating system is not Windows Vista or later</exception>
    public TaskbarProgressBar(nint Handle)
    {
        if (Handle < 1) { throw new ArgumentOutOfRangeException("Invalid Handle 0x" + Handle.ToString("X")); }

        if (Environment.OSVersion.Platform != PlatformID.Win32NT || Environment.OSVersion.Version.Major < 6)
        {
            throw new NotImplementedException("The progress bar in taskbar feature is only available on Windows Vista or later");
        }

        handle = Handle;
        taskbar = new CTaskbarList() as ITaskbarList3;
        taskbar.HrInit();

        taskbar.SetProgressState(handle, 0x00);
        MaximumValue = 100;
    }

    /// <summary>
    /// Change the "Status" of a WinForms progress bar.
    /// </summary>
    /// <param name="progressBar">The progress bar control</param>
    /// <param name="state">The new state. Note: the ProgressBarState.Marquee is useless in this method.</param>
    public static void SetProgressBarState(ProgressBar progressBar, ProgressBarState state)
    {
        if (state == ProgressBarState.Marquee)
        {
            progressBar.Style = ProgressBarStyle.Marquee;
            return;
        }

        int value;
        switch (state)
        {
            default:
                value = 0x01;//ProgressBarState.Normal
                break;
            case ProgressBarState.Error:
                value = 0x02;
                break;
            case ProgressBarState.WarningOrPause:
                value = 0x03;
                break;
        }

        //progressBar.Style = ProgressBarStyle.Blocks;

        SendMessage(progressBar.Handle, PBM_SETSTATE, (nint)value, 0x00);
    }

    /// <summary>
    /// Gets or sets the current progress state.
    /// </summary>
    public ProgressBarState State
    {
        get { return (ProgressBarState)current_state; }
        set
        {
            int val = (int)value;
            taskbar.SetProgressState(handle, val);
            Interlocked.Exchange(ref current_state, val);
        }
    }

    /// <summary>
    /// Gets or set the current progress value.
    /// </summary>
    public int Value
    {
        get { return current_progress; }
        set
        {
            value = Math.Min(MaximumValue, Math.Max(0, value));
            taskbar.SetProgressValue(handle, (ulong)value, (ulong)MaximumValue);
            Interlocked.Exchange(ref current_progress, value);
        }
    }

    /// <summary>
    /// Gets or sets a value that represents whether the ProgressBar is visible on taskbar.
    /// </summary>
    public bool Visible
    {
        get { return is_enabled; }
        set
        {
            if (is_enabled == value) { return; }
            int val = value ? current_state : 0x00;

            taskbar.SetProgressState(handle, val);
            is_enabled = value;
        }
    }

    /// <summary>
    /// Gets or sets the maximum value, the maximum by default is 100.
    /// </summary>
    public int MaximumValue { get; set; }

    [DllImport("user32", CharSet = CharSet.Auto, SetLastError = false)]
    private static extern nint SendMessage(nint hWnd, uint Msg, nint wParam, nint lParam);

    private const int PBM_SETSTATE = 0x0410;
    private int current_progress = 0x00;
    private int current_state = (int)ProgressBarState.Normal;
    private volatile bool is_enabled = false;
    private static ITaskbarList3 taskbar;
    private nint handle;

    /// <summary>
    /// Represents the progress bar state, this changes the color based on the theme.
    /// </summary>
    [Flags]
    public enum ProgressBarState
    {
        /// <summary>
        /// Indicates progress by moving a block continuously by the marquee style in green color.
        /// </summary>
        Marquee = 1,
        /// <summary>
        /// Green color.
        /// </summary>
        Normal = 2,
        /// <summary>
        /// Red color.
        /// </summary>
        Error = 4,
        /// <summary>
        /// Yellow color
        /// </summary>
        WarningOrPause = 8
    }

    [ComImportAttribute()]
    [GuidAttribute("ea1afb91-9e28-4b86-90e9-9e9f8a5eefaf")]
    [InterfaceTypeAttribute(ComInterfaceType.InterfaceIsIUnknown)]
    private interface ITaskbarList3
    {
        [PreserveSig] void HrInit();
        [PreserveSig] void AddTab(nint hwnd);
        [PreserveSig] void DeleteTab(nint hwnd);
        [PreserveSig] void ActivateTab(nint hwnd);
        [PreserveSig] void SetActiveAlt(nint hwnd);
        [PreserveSig] void MarkFullscreenWindow(nint hwnd, [MarshalAs(UnmanagedType.Bool)] bool fFullscreen);
        [PreserveSig] void SetProgressValue(nint hwnd, UInt64 ullCompleted, UInt64 ullTotal);
       [PreserveSig]  void SetProgressState(nint hwnd, int tbpFlags);
    }

    [Guid("56FDF344-FD6D-11d0-958A-006097C9A090")]
    [ClassInterfaceAttribute(ClassInterfaceType.None)]
    [ComImportAttribute()]
    private class CTaskbarList { }
}
