using System;
using System.Diagnostics;
using System.Net.Http;
using System.Threading.Tasks;
using Newtonsoft.Json.Linq;

class Program
{
    static readonly HttpClient client = new HttpClient();

    static async Task Main(string[] args)
    {
        var stopwatch = new Stopwatch();
        stopwatch.Start();

        var tasks = new Task[10];
        for (int i = 0; i < 10; i++)
        {
            tasks[i] = DownloadImageAsync(i + 1);
        }

        await Task.WhenAll(tasks);

        stopwatch.Stop();
        Console.WriteLine($"Time: {stopwatch.ElapsedMilliseconds} ms");
    }

    static async Task DownloadImageAsync(int requestNumber)
    {
        Console.WriteLine($"Start {requestNumber} download...");

        var response = await client.GetAsync("https://dog.ceo/api/breeds/image/random");
        response.EnsureSuccessStatusCode();
        var content = await response.Content.ReadAsStringAsync();

        // Парсинг JSON для извлечения URL изображения
        var json = JObject.Parse(content);
        var imageUrl = (string)json["message"];

        var imageResponse = await client.GetAsync(imageUrl);
        var imageBytes = await imageResponse.Content.ReadAsByteArrayAsync();

        var fileName = $"image{DateTime.Now.Ticks}.jpg";
        await System.IO.File.WriteAllBytesAsync(fileName, imageBytes);

        Console.WriteLine("Downloaded image");
    }
}