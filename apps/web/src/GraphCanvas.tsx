import { useEffect, useRef, useState } from 'react';
import {
  ReactFlow,
  Background,
  MiniMap,
  Handle,
  Position,
  applyNodeChanges,
  applyEdgeChanges,
  type Node,
  type NodeProps,
  type Edge,
  type Connection,
  type ReactFlowInstance,
} from '@xyflow/react';
import type { NodeTiming } from '@smartflow/runtime';
import type { GraphDocument, NodeDocument } from '@smartflow/core';
import type { NodeDefinition } from '@smartflow/extension-sdk';
import type { GraphWorkspace, Point } from './workspace.ts';
import '@xyflow/react/dist/style.css';

type CanvasNode = Node<
  {
    label: string;
    packageId: string;
    inputs: string[];
    outputs: string[];
    supported: boolean;
    available: boolean;
    status?: string;
    duration?: string;
  },
  'smartflow'
>;
function SmartNode({ data, selected }: NodeProps<CanvasNode>) {
  return (
    <div
      className={`graph-node ${selected ? 'selected' : ''} ${data.supported ? '' : 'unsupported'}`}
    >
      <div className="node-heading">
        <span className={`node-dot ${data.packageId.includes('scene') ? 'scene' : 'data'}`} />
        <strong>{data.label}</strong>
      </div>
      <div className="node-package">{data.packageId}</div>
      <div className="node-ports">
        <div>
          {data.inputs.map((id) => (
            <div className="port-row" key={id}>
              <Handle
                type="target"
                id={id}
                position={Position.Left}
                isConnectable={data.supported}
              />
              <span>{id}</span>
            </div>
          ))}
        </div>
        <div>
          {data.outputs.map((id) => (
            <div className="port-row output" key={id}>
              <span>{id}</span>
              <Handle
                type="source"
                id={id}
                position={Position.Right}
                isConnectable={data.supported}
              />
            </div>
          ))}
        </div>
      </div>
      {data.duration && (
        <div
          className="node-time"
          title="Invocation wall time; excludes ready-queue waiting. Components include their complete body span."
        >
          Run: {data.duration} ms
        </div>
      )}
      <div className={`node-status ${data.status ?? ''}`}>
        {data.status ??
          (data.supported
            ? data.available
              ? 'Ready'
              : 'Execution pending'
            : 'Unavailable · content retained')}
      </div>
    </div>
  );
}
const nodeTypes = { smartflow: SmartNode };
interface Props {
  graph: GraphDocument;
  workspace: GraphWorkspace;
  definition(node: NodeDocument): NodeDefinition | undefined;
  onConnect(connection: Connection): void;
  onSelection(nodes: string[], edges: string[]): void;
  onPositions(positions: Record<string, Point>): void;
  onViewport(viewport: { x: number; y: number; zoom: number }): void;
  statuses?: Record<string, string>;
  timings?: Record<string, NodeTiming>;
  elapsedMs?: number;
}
function projectNodes(props: Props): CanvasNode[] {
  return props.graph.nodes.map((node, index) => {
    const definition = props.definition(node);
    const timing = props.timings?.[node.id];
    const inferredInputs = props.graph.connections
      .filter((edge) => edge.target.nodeId === node.id)
      .map((edge) => edge.target.portId);
    const inferredOutputs = props.graph.connections
      .filter((edge) => edge.source.nodeId === node.id)
      .map((edge) => edge.source.portId);
    return {
      id: node.id,
      type: 'smartflow',
      position: props.workspace.positions[node.id] ?? {
        x: (index % 4) * 260,
        y: Math.floor(index / 4) * 180,
      },
      selected: props.workspace.selection.includes(node.id),
      data: {
        label: definition?.label ?? node.typeId,
        packageId: node.packageId,
        supported: !!definition,
        available: definition?.capabilities.includes('browser') ?? false,
        inputs: [...new Set(definition?.inputs.map((port) => port.id) ?? inferredInputs)],
        outputs: [...new Set(definition?.outputs.map((port) => port.id) ?? inferredOutputs)],
        ...(timing?.startedMs !== undefined
          ? {
              duration: (
                (timing.endedMs ?? props.elapsedMs ?? timing.startedMs) - timing.startedMs
              ).toFixed(3),
            }
          : {}),
        ...(props.statuses?.[node.id] ? { status: props.statuses[node.id]! } : {}),
      },
    };
  });
}
export function GraphCanvas(props: Props) {
  const root = useRef<HTMLDivElement>(null);
  const instance = useRef<ReactFlowInstance<CanvasNode, Edge> | null>(null);
  const [nodes, setNodes] = useState<CanvasNode[]>(() => projectNodes(props));
  const [edges, setEdges] = useState<Edge[]>([]);
  useEffect(() => {
    setNodes(projectNodes(props));
    const ids = new Set(props.graph.nodes.map((node) => node.id));
    setEdges((previous) =>
      props.graph.connections
        .filter((edge) => ids.has(edge.source.nodeId) && ids.has(edge.target.nodeId))
        .map((edge) => ({
          id: edge.id,
          source: edge.source.nodeId,
          sourceHandle: edge.source.portId,
          target: edge.target.nodeId,
          targetHandle: edge.target.portId,
          selected: previous.find((item) => item.id === edge.id)?.selected ?? false,
          type: 'smoothstep',
          style: { stroke: '#749b9a', strokeWidth: 2 },
        })),
    );
  }, [
    props.graph,
    props.workspace.positions,
    props.workspace.selection,
    props.definition,
    props.statuses,
    props.timings,
    props.elapsedMs,
  ]);
  const saveViewport = () => {
    if (instance.current) props.onViewport(instance.current.getViewport());
  };
  return (
    <div className="canvas" ref={root} aria-label="Graph canvas">
      <ReactFlow<CanvasNode, Edge>
        nodes={nodes}
        edges={edges}
        nodeTypes={nodeTypes}
        colorMode="dark"
        onlyRenderVisibleElements
        minZoom={0.01}
        maxZoom={2}
        onNodesChange={(changes) => {
          const next = applyNodeChanges(
            changes.filter((change) => change.type !== 'remove'),
            nodes,
          );
          setNodes(next);
          if (changes.some((change) => change.type === 'select'))
            props.onSelection(
              next.filter((node) => node.selected).map((node) => node.id),
              edges.filter((edge) => edge.selected).map((edge) => edge.id),
            );
        }}
        onEdgesChange={(changes) => {
          const next = applyEdgeChanges(
            changes.filter((change) => change.type !== 'remove'),
            edges,
          );
          setEdges(next);
          if (changes.some((change) => change.type === 'select'))
            props.onSelection(
              nodes.filter((node) => node.selected).map((node) => node.id),
              next.filter((edge) => edge.selected).map((edge) => edge.id),
            );
        }}
        onConnect={props.onConnect}
        deleteKeyCode={null}
        onNodeDragStop={(_, __, dragged) =>
          props.onPositions(Object.fromEntries(dragged.map((node) => [node.id, node.position])))
        }
        onMoveEnd={(event, viewport) => {
          if (event) props.onViewport(viewport);
        }}
        onInit={(value) => {
          instance.current = value;
          if (props.workspace.viewport) void value.setViewport(props.workspace.viewport);
          else if (props.workspace.nativeCenter) {
            const { x, y, scale } = props.workspace.nativeCenter;
            void value.setViewport({
              x: (root.current?.clientWidth ?? 800) / 2 - x * scale,
              y: (root.current?.clientHeight ?? 500) / 2 - y * scale,
              zoom: scale,
            });
          } else void value.fitView({ padding: 0.25 });
        }}
      >
        <Background gap={24} size={1} color="#344044" />
        <MiniMap pannable zoomable nodeColor="#487e79" />
      </ReactFlow>
      <div className="canvas-controls">
        <button
          aria-label="Zoom in"
          onClick={() => {
            void instance.current?.zoomIn({ duration: 0 }).then(saveViewport);
          }}
        >
          +
        </button>
        <button
          aria-label="Zoom out"
          onClick={() => {
            void instance.current?.zoomOut({ duration: 0 }).then(saveViewport);
          }}
        >
          −
        </button>
        <button
          onClick={() => {
            void instance.current?.fitView({ padding: 0.25, duration: 0 }).then(saveViewport);
          }}
        >
          Fit graph
        </button>
      </div>
      {!props.graph.nodes.length && (
        <div className="canvas-empty">
          <strong>Your workspace starts here</strong>
          <span>Add an operation from the library, then connect its ports.</span>
        </div>
      )}
    </div>
  );
}
