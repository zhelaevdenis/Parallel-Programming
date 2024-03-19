using System;
using System.Diagnostics;
using System.Net.Http;
using System.Threading.Tasks;

class Program
{
    static readonly HttpClient client = new HttpClient();

    static async Task DownloadImageAsync()
    {
        Console.WriteLine("stat downloading");
        var response = await client.GetAsync("https://dog.ceo/api/breeds/image/random");
        response.EnsureSuccessStatusCode();
        var content = await response.Content.ReadAsStringAsync();
        Console.WriteLine("Downloaded image");
    }

    static async Task Main(string[] args)
    {
        Stopwatch stopwatch = new Stopwatch();
        stopwatch.Start();

        for (int i = 0; i < 10; i++)
        {
            await DownloadImageAsync();
        }

        stopwatch.Stop();
        Console.WriteLine($"Time: {stopwatch.ElapsedMilliseconds} ms");
    }

    
}