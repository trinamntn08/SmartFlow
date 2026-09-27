export function App() {
  return (
    <main>
      <header>
        <span className="mark" aria-hidden="true">
          VW
        </span>
        <span>Visual Workspace</span>
        <span className="badge">Development starter</span>
      </header>
      <section className="intro" aria-labelledby="title">
        <p className="eyebrow">Connect. Explore. Extend.</p>
        <h1 id="title">
          One workspace.
          <br />
          Many possibilities.
        </h1>
        <p className="lead">
          Build interactive tools with node graphs. Start with 3D scenes and extend into other
          fields.
        </p>
      </section>
      <section className="domains" aria-label="Planned capabilities">
        <article>
          <span className="number">01 / FOUNDATION</span>
          <h2>Interactive graphs</h2>
          <p>Connect operations, adjust parameters, and inspect each result.</p>
        </article>
        <article>
          <span className="number">02 / INCLUDED DOMAIN</span>
          <h2>3D scenes</h2>
          <p>Compose geometry, materials, and transforms in an interactive scene workspace.</p>
        </article>
        <article>
          <span className="number">03 / EXTENSIONS</span>
          <h2>Room to grow</h2>
          <p>Add new types, operations, and viewers for data, images, or another domain.</p>
        </article>
      </section>
      <footer>
        The local development environment is ready. Graph editing, execution, and viewers are the
        next implementation steps.
      </footer>
    </main>
  );
}
